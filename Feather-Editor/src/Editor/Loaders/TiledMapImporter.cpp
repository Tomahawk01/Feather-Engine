#include "TiledMapImporter.h"

#include "Core/CoreUtils/CoreEngineData.h"
#include "Core/CoreUtils/CoreUtilities.h"
#include "Core/Resources/AssetManager.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MainRegistry.h"

#include "Renderer/Essentials/Texture.h"

#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"

#include <tinyxml2.h>

using namespace tinyxml2;

namespace Feather {

	bool TiledMapImporter::ImportTilemapFromTiled(EditorSceneManager* sceneManager, const std::string tiledMapFile)
	{
		fs::path filepath{ tiledMapFile };
		if (filepath.extension() == ".lua")
			return ImportFromLuaFile(sceneManager, tiledMapFile);
		else if (filepath.extension() == ".tmx")
			return ImportFromTMXFile(sceneManager, tiledMapFile);

		F_ERROR("Failed to import tiled map. File '{}' is not supported", tiledMapFile);
		return false;
	}

	bool TiledMapImporter::ImportFromTMXFile(EditorSceneManager* sceneManager, const std::string tiledMapFile)
	{
		auto XMLDoc = std::make_unique<tinyxml2::XMLDocument>();
		if (!XMLDoc)
			return false;

		if (XMLDoc->LoadFile(tiledMapFile.c_str()) != XML_SUCCESS)
		{
			std::string sError{ XMLDoc->ErrorStr() };
			F_ERROR("Failed to load tiled map file '{}': {}", tiledMapFile, sError);
			return false;
		}

		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		SceneObject newScene{ fs::path{ tiledMapFile }.stem().string(), EMapType::Grid };
		auto& canvas = newScene.GetCanvas();

		// Get the Root Element
		XMLElement* rootElement = XMLDoc->RootElement();
		if (!rootElement)
		{
			std::string sError{ XMLDoc->ErrorStr() };
			F_ERROR("Failed to load tiled map file '{}': {}", tiledMapFile, sError);
			return false;
		}

		XMLElement* tileset = rootElement->FirstChildElement("tileset");
		if (!tileset)
		{
			std::string sError{ XMLDoc->ErrorStr() };
			F_ERROR("Failed to load tiled map file '{}': {}", tiledMapFile, sError);
			return false;
		}

		// Get all tilesets
		std::vector<Tileset> tilesets;

		// Load all of the tileset info
		while (tileset)
		{
			std::vector<Tile> tiles;
			XMLElement* tile = tileset->FirstChildElement("tile");
			while (tile)
			{
				Tile newTile{};
				newTile.id = std::atoi(tile->Attribute("id"));

				XMLElement* objectGroup = tile->FirstChildElement("objectgroup");
				XMLElement* object = objectGroup->FirstChildElement("object");

				while (object)
				{
					TileObject tileObject{};
					tileObject.name = object->Attribute("name") ? object->Attribute("name") : "";
					tileObject.type = object->Attribute("type") ? object->Attribute("type") : "";
					tileObject.width = std::atoi(object->Attribute("width"));
					tileObject.height = std::atoi(object->Attribute("height"));
					tileObject.position = glm::vec2{ std::atof(object->Attribute("x")), std::atof(object->Attribute("x")) };

					newTile.tileObjects.emplace_back(std::move(tileObject));
					object = object->NextSiblingElement("object");
				}

				tiles.emplace_back(std::move(newTile));

				// Move to the next tile
				tile = tile->NextSiblingElement("tile");
			}

			Tileset newTileset{
				.name = tileset->Attribute("name") ? tileset->Attribute("name") : "",
				.columns = tileset->Attribute("columns") ? std::atoi(tileset->Attribute("columns")) : -1,
				.width = tileset->Attribute("width") ? std::atoi(tileset->Attribute("width")) : 0,
				.height = tileset->Attribute("height") ? std::atoi(tileset->Attribute("height")) : 0,
				.tileWidth = tileset->Attribute("tilewidth") ? std::atoi(tileset->Attribute("tilewidth")) : 16,
				.tileHeight = tileset->Attribute("tileheight") ? std::atoi(tileset->Attribute("tileheight")) : 16,
				.firstGID = tileset->Attribute("firstgid") ? std::atoi(tileset->Attribute("firstgid")) : 1,
				.tileCount = tileset->Attribute("tilecount") ? std::atoi(tileset->Attribute("tilecount")) : 1,
				.tiles = std::move(tiles)
			};

			if (newTileset.name.empty() || newTileset.columns <= 0)
			{
				F_ERROR("Failed to load tileset");
				return false;
			}

			newTileset.SetRows();
			tilesets.emplace_back(std::move(newTileset));

			// Move to the next tileset
			tileset = tileset->NextSiblingElement("tileset");
		}

		XMLElement* layer = rootElement->FirstChildElement("layer");
		int LAYER = 0;
		while (layer)
		{
			const int ROWS = std::atoi(layer->Attribute("height")) - 1;
			const int COLS = std::atoi(layer->Attribute("width"));
			const std::string sLayerName = layer->Attribute("name");

			newScene.AddLayer(SpriteLayerParams{
				.layerName = sLayerName,
				.isVisible = true,
				.layer = LAYER
			});

			XMLElement* layerData = layer->FirstChildElement("data");
			const char* dataCSV = layerData->GetText();

			std::vector<int> tileIDs;
			std::stringstream ss(dataCSV);
			std::string value;

			// Parse the data
			while (std::getline(ss, value, ','))
			{
				if (!value.empty())
				{
					tileIDs.push_back(std::stoi(value));
				}
			}

			for (int row = 0; row < ROWS; row++)
			{
				for (int col = 1; col < COLS; col++)
				{
					int id = tileIDs[row * COLS + col];
					// If the ID is zero, we can skip, there is no tile there
					if (id == 0)
						continue;

					if (auto* tileset = GetTileset(tilesets, id))
					{
						Entity newTile{ newScene.GetRegistryPtr(), "", "" };
						auto& transform = newTile.AddComponent<TransformComponent>();
						transform.position = glm::vec2{ (col - 1) * tileset->tileWidth, row * tileset->tileHeight };

						// Add the box collider if there is an object
						if (auto* object = tileset->GetObjectFromId(id))
						{
							auto& boxCollider = newTile.AddComponent<BoxColliderComponent>();
							boxCollider.width = object->width;
							boxCollider.height = object->height;
							boxCollider.offset = object->position;

							if (coreGlobals.IsPhysicsEnabled())
							{
								PhysicsAttributes physAttr{};
								physAttr.eType = RigidBodyType::STATIC;
								physAttr.density = 1000.0f;
								physAttr.friction = 0.0f;
								physAttr.restitution = 0.0f;
								physAttr.position = transform.position + object->position;
								physAttr.isFixedRotation = true;
								physAttr.boxSize = glm::vec2{ boxCollider.width, boxCollider.height };
								physAttr.isBoxShape = true;

								ObjectData objectData{};
								objectData.tag = object->name;
								objectData.group = object->type;
								objectData.entityID = static_cast<uint32_t>(newTile.GetEntity());

								newTile.AddComponent<PhysicsComponent>(PhysicsComponent{ physAttr });
							}
						}
						auto texture = assetManager.GetTexture(tileset->name);
						F_ASSERT(texture && "All tiles must have a texture. Texture Must be loaded into asset manager before import");
						if (!texture)
						{
							F_ERROR("Failed to import tiled map. Texture '{}' must be loaded in the asset manager prior to import", tileset->name);
							return false;
						}

						auto [startX, startY] = tileset->GetTileStartXY(id);

						auto& sprite = newTile.AddComponent<SpriteComponent>();
						sprite.textureName = tileset->name;
						sprite.width = tileset->tileWidth;
						sprite.height = tileset->tileHeight;
						sprite.start_x = startX;
						sprite.start_y = startY;
						sprite.layer = LAYER;

						GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());

						newTile.AddComponent<TileComponent>(
							TileComponent{ .id = static_cast<uint32_t>(newTile.GetEntity()) });
					}
				}
			}

			// Move to the next layer
			layer = layer->NextSiblingElement("layer");
			LAYER++;
		}

		newScene.SaveScene(true);
		return sceneManager->AddSceneObject(newScene.GetSceneName(), newScene.GetSceneDataPath());
	}

	bool TiledMapImporter::ImportFromLuaFile(EditorSceneManager* sceneManager, const std::string tiledMapFile)
	{
		auto& coreGlobals = CORE_GLOBALS();
		auto& mainRegistry = MAIN_REGISTRY();
		auto& assetManager = mainRegistry.GetAssetManager();

		sol::state lua;
		lua.open_libraries(sol::lib::base);
		sol::table map;
		try
		{
			map = lua.safe_script_file(tiledMapFile);
		}
		catch (const sol::error& err)
		{
			F_ERROR("Failed to import Tiled map. {}", err.what());
			return false;
		}

		if (!map.valid())
		{
			F_ERROR("Failed to import Tiled map. Map table invalid");
			return false;
		}

		SceneObject newScene{ fs::path{ tiledMapFile }.stem().string(), EMapType::Grid };
		auto& canvas = newScene.GetCanvas();

		canvas.tileWidth = map["tilewidth"].get_or(16);
		canvas.tileHeight = map["tileheight"].get_or(16);
		canvas.width = map["width"].get_or(20) * canvas.tileWidth;

		// Get all tilesets
		std::vector<Tileset> tilesets;
		sol::optional<sol::table> optTilesets = map["tilesets"];
		if (optTilesets)
		{
			for (const auto& [_, tileset] : *optTilesets)
			{
				sol::table tilesetTbl = tileset.as<sol::table>();
				sol::optional<sol::table> optTilesetTiles = tilesetTbl["tiles"];
				std::vector<Tile> tiles;

				if (optTilesetTiles)
				{
					for (const auto& [index, tileObj] : *optTilesetTiles)
					{
						sol::table tileTbl = tileObj.as<sol::table>();
						Tile newTile{};
						newTile.id = tileTbl["id"].get<int>();
						sol::table objects = tileTbl["objectGroup"]["objects"].get<sol::table>();
						for (const auto& [objIndex, objectObj] : objects)
						{
							sol::table objectTbl = objectObj.as<sol::table>();
							TileObject tileObject{
								.name = objectTbl["name"].get_or(std::string{}),
								.type = objectTbl["type"].get_or(std::string{}),
								.position = glm::vec2{ objectTbl["x"].get_or(0.0f), objectTbl["y"].get_or(0.0f) },
								.width = objectTbl["width"].get_or(16.0f),
								.height = objectTbl["height"].get_or(16.0f),
								.rotation = objectTbl["rotation"].get_or(0.0f) };

							newTile.tileObjects.emplace_back(std::move(tileObject));
						}

						tiles.emplace_back(std::move(newTile));
					}
				}

				Tileset newTileset{ .name = tilesetTbl["name"].get_or(std::string{}),
									.columns = tilesetTbl["columns"].get_or(-1),
									.width = tilesetTbl["imagewidth"].get_or(0),
									.height = tilesetTbl["imageheight"].get_or(0),
									.tileWidth = tilesetTbl["tilewidth"].get_or(16),
									.tileHeight = tilesetTbl["tileheight"].get_or(16),
									.firstGID = tilesetTbl["firstgid"].get_or(1),
									.tileCount = tilesetTbl["tilecount"].get_or(1),
									.tiles = std::move(tiles) };

				if (newTileset.name.empty() || newTileset.columns <= 0)
				{
					F_ERROR("Failed to load tileset");
					return false;
				}

				newTileset.SetRows();
				tilesets.emplace_back(newTileset);
			}
		}

		sol::optional<sol::table> optMapLayers = map["layers"];
		if (optMapLayers)
		{
			for (const auto& [index, layerObj] : *optMapLayers)
			{
				sol::table layer = layerObj.as<sol::table>();
				const int ROWS = layer["height"].get<int>() - 1;
				const int COLS = layer["width"].get<int>();
				const int LAYER = index.as<int>();

				const std::string sLayerName = layer["name"].get<std::string>();
				bool bVisible = layer["visible"].get<bool>();

				newScene.AddLayer(SpriteLayerParams{
					.layerName = sLayerName,
					.isVisible = bVisible,
					.layer = LAYER,
				});

				sol::table layerData = layer["data"];

				for (int row = 0; row < ROWS; row++)
				{
					for (int col = 1; col < COLS; col++)
					{
						int id = layerData[row * COLS + col].get<int>();
						if (id == 0)
							continue;

						if (auto* tileset = GetTileset(tilesets, id))
						{
							Entity newTile{ newScene.GetRegistryPtr(), "", "" };
							auto& transform = newTile.AddComponent<TransformComponent>();
							transform.position = glm::vec2{ (col - 1) * tileset->tileWidth, row * tileset->tileHeight };

							// Add the box collider if there is an object
							if (auto* object = tileset->GetObjectFromId(id))
							{
								auto& boxCollider = newTile.AddComponent<BoxColliderComponent>();
								boxCollider.width = object->width;
								boxCollider.height = object->height;
								boxCollider.offset = object->position;

								if (coreGlobals.IsPhysicsEnabled())
								{
									PhysicsAttributes physAttr{};
									physAttr.eType = RigidBodyType::STATIC;
									physAttr.density = 1000.0f;
									physAttr.friction = 0.0f;
									physAttr.restitution = 0.0f;
									physAttr.position = transform.position + object->position;
									physAttr.isFixedRotation = true;
									physAttr.boxSize = glm::vec2{ boxCollider.width, boxCollider.height };
									physAttr.isBoxShape = true;

									ObjectData objectData{};
									objectData.tag = object->name;
									objectData.group = object->type;
									objectData.entityID = static_cast<uint32_t>(newTile.GetEntity());

									newTile.AddComponent<PhysicsComponent>(PhysicsComponent{ physAttr });
								}
							}
							auto texture = assetManager.GetTexture(tileset->name);
							F_ASSERT(texture && "All tiles must have a texture. Texture must be loaded into asset manager before import");
							if (!texture)
							{
								F_ERROR("Failed to import tiled map. Texture '{}' must be loaded in the asset manager prior to import", tileset->name);
								return false;
							}

							auto [startX, startY] = tileset->GetTileStartXY(id);

							auto& sprite = newTile.AddComponent<SpriteComponent>();
							sprite.textureName = tileset->name;
							sprite.width = tileset->tileWidth;
							sprite.height = tileset->tileHeight;
							sprite.start_x = startX;
							sprite.start_y = startY;
							sprite.layer = LAYER;

							GenerateUVs(sprite, texture->GetWidth(), texture->GetHeight());

							newTile.AddComponent<TileComponent>(TileComponent{ .id = static_cast<uint32_t>(newTile.GetEntity()) });
						}
					}
				}
			}
		}

		newScene.SaveScene(true);

		return sceneManager->AddSceneObject(newScene.GetSceneName(), newScene.GetSceneDataPath());
	}

	TiledMapImporter::Tileset* TiledMapImporter::GetTileset(std::vector<Tileset>& tilesets, int id)
	{
		for (int i = 0; i < tilesets.size(); i++)
		{
			auto& tileset = tilesets[i];
			if (tileset.TileIdExists(id))
			{
				return &tileset;
			}
		}

		return nullptr;
	}

	std::tuple<int, int> TiledMapImporter::Tileset::GetTileStartXY(int id)
	{
		if (!TileIdExists(id))
			return std::make_tuple(-1, -1);

		int tileID = id - firstGID;
		int startY = tileID / columns;
		int startX = tileID % columns;

		return std::make_tuple(startX, startY);
	}

	TiledMapImporter::TileObject* TiledMapImporter::Tileset::GetObjectFromId(int id)
	{
		int actualID = id - firstGID;
		for (auto& tile : tiles)
		{
			if (tile.id == actualID)
			{
				if (!tile.tileObjects.empty())
				{
					// Return the first object
					return &tile.tileObjects[0];
				}
			}
		}

		return nullptr;
	}

}
