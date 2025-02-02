#pragma once
#include "Core/ECS/Components/AllComponents.h"
#include "Physics/UserData.h"

namespace Feather {

	class Texture;

	constexpr const char* DROP_TEXTURE_SRC = "DropTextureSource";
	constexpr const char* DROP_FONT_SRC = "DropFontSource";
	constexpr const char* DROP_MUSIC_SRC = "DropMusicSource";
	constexpr const char* DROP_SOUNDFX_SRC = "DropSoundFXSource";
	constexpr const char* DROP_SCENE_SRC = "DropSceneSource";

#define BASE_PATH                                                                                                      \
	std::string                                                                                                        \
	{                                                                                                                  \
		SDL_GetBasePath()                                                                                              \
	}
#ifdef _WIN32
	constexpr char PATH_SEPARATOR = '\\';
#define DEFAULT_PROJECT_PATH BASE_PATH + "FeatherProjects"
#else
	constexpr char PATH_SEPARATOR = '/';
#define DEFAULT_PROJECT_PATH BASE_PATH + PATH_SEPARATOR + "FeatherProjects"
#endif

#define SCRIPTS "scripts"
#define ASSETS "assets" 

	constexpr const char* FEATHER_PRJ_FILE_EXT = ".fprj";

	struct Canvas
	{
		int width{ 640 }, height{ 480 };
		int tileWidth{ 16 }, tileHeight{ 16 };
	};

	struct Tile
	{
		uint32_t id{ entt::null };
		TransformComponent transform{};
		SpriteComponent sprite{};
		AnimationComponent animation{};
		BoxColliderComponent boxCollider{};
		CircleColliderComponent circleCollider{};
		PhysicsComponent physics{};
		ObjectData objectData{};

		bool isCollider{ false }, hasAnimation{ false }, hasPhysics{ false }, isCircle{ false };
	};

	constexpr Color XAXIS_GIZMO_COLOR = { 255, 0, 0, 175 };
	constexpr Color XAXIS_HOVERED_GIZMO_COLOR = { 255, 255, 0, 175 };
	constexpr Color YAXIS_GIZMO_COLOR = { 0, 255, 0, 175 };
	constexpr Color YAXIS_HOVERED_GIZMO_COLOR = { 0, 255, 255, 175 };
	constexpr Color GRAYED_OUT_GIZMO_COLOR = { 135, 135, 135, 175 };

	struct GizmoAxisParams
	{
		TransformComponent transform{};
		SpriteComponent sprite{};
		glm::vec2 axisOffset;

		Color axisColor;
		Color axisHoveredColor;
		Color axisDisabledColor;
	};

	enum class FileType
	{
		SOUND,
		IMAGE,
		TXT, // All text file types
		FOLDER,

		INVALID_TYPE
	};

	FileType GetFileType(const std::string& sPath);

	std::vector<std::string> SplitStr(const std::string& str, char delimiter);
	Texture* GetIconTexture(const std::string& sPath);

}
