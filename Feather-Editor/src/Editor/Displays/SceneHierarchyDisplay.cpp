#include "SceneHierarchyDisplay.h"

#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MetaUtilities.h"
#include "Core/Events/EventDispatcher.h"
#include "Core/Events/EngineEventTypes.h"
#include "Core/Scripting/InputManager.h"
#include "Core/CoreUtils/Prefab.h"
#include "Core/Resources/AssetManager.h"

#include "Editor/Scene/SceneManager.h"
#include "Editor/Scene/SceneObject.h"
#include "Editor/Tools/ToolManager.h"
#include "Editor/Utilities/GUI/ImGuiUtils.h"
#include "Editor/Utilities/Fonts/IconsFontAwesome5.h"
#include "Editor/Events/EditorEventTypes.h"

#include <imgui.h>

#include <format>

using namespace entt::literals;

namespace Feather {

	SceneHierarchyDisplay::SceneHierarchyDisplay()
	{
		ADD_SWE_HANDLER(SwitchEntityEvent, &SceneHierarchyDisplay::OnEntityChanged, *this);
		ADD_EVENT_HANDLER(KeyEvent, &SceneHierarchyDisplay::OnKeyPressed, *this);
		ADD_EVENT_HANDLER(AddComponentEvent, &SceneHierarchyDisplay::OnAddComponent, *this);
	}

	SceneHierarchyDisplay::~SceneHierarchyDisplay() = default;

	void SceneHierarchyDisplay::Update()
	{
	}

	void SceneHierarchyDisplay::Draw()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentSceneObject();
		if (!ImGui::Begin(ICON_FA_SITEMAP " Scene Hierarchy") || !currentScene)
		{
			ImGui::End();
			return;
		}

		m_WindowActive = ImGui::IsWindowFocused();

		OpenContext(currentScene);

		ImGui::Separator();
		ImGui::AddSpaces(1);
		ImGui::InlineLabel(ICON_FA_SEARCH, 32.0f);
		m_TextFilter.Draw("##search_filter");
		ImGui::AddSpaces(1);
		ImGui::Separator();

		auto& registry = currentScene->GetRegistry();
		auto sceneEntities = registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent, ScriptComponent>);

		for (auto entity : sceneEntities)
		{
			Entity ent{ registry, entity };
			if (!m_TextFilter.PassFilter(ent.GetName().c_str()))
				continue;

			// Do not redraw the children
			const auto& relations = ent.GetComponent<Relationship>();
			if (relations.parent == entt::null)
			{
				if (OpenTreeNode(ent))
					ImGui::TreePop();
			}
		}

		ImGui::End();

		DrawGameObjectDetails();
	}

	bool SceneHierarchyDisplay::OpenTreeNode(Entity& entity)
	{
		const auto& name = entity.GetName();

		ImGui::PushID(static_cast<int32_t>(entity.GetEntity()));

		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding |
									   ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		// Get a copy of the relationships of the current entity
		auto relations = entity.GetComponent<Relationship>();
		auto curr = relations.firstChild;

		bool selected{ m_SelectedEntity && m_SelectedEntity->GetEntity() == entity.GetEntity() };
		if (selected)
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool treeNodeOpen{ false };
		treeNodeOpen = ImGui::TreeNodeEx(std::format("{} {}", ICON_FA_CUBE, name).c_str(), nodeFlags);
		auto currentScene = SCENE_MANAGER().GetCurrentScene();

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = std::make_shared<Entity>(currentScene->GetRegistry(), entity.GetEntity());
			TOOL_MANAGER().SetSelectedEntity(entity.GetEntity());
		}

		ImGui::PopID();

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("SceneHierarchy", &(entity.GetEntity()), sizeof(entity.GetEntity()));
			ImGui::Text(entity.GetName().c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchy");
			if (payload)
			{
				F_ASSERT(payload->DataSize == sizeof(entity.GetEntity()));
				entt::entity* ent = (entt::entity*)payload->Data;
				auto entID = static_cast<int32_t>(*ent);
				entity.AddChild(*ent);
			}
			ImGui::EndDragDropTarget();
		}

		while (curr != entt::null && treeNodeOpen)
		{
			Entity ent{ currentScene->GetRegistry(), curr };
			if (OpenTreeNode(ent))
				ImGui::TreePop();
			curr = ent.GetComponent<Relationship>().nextSibling;
		}

		return treeNodeOpen;
	}

	void SceneHierarchyDisplay::AddComponent(Entity& entity, bool* addComponent)
	{
		if (!addComponent)
			return;

		if (*addComponent)
			ImGui::OpenPopup("Add Component");

		if (ImGui::BeginPopupModal("Add Component"))
		{
			auto& registry = entity.GetEnttRegistry();

			// Get all of the reflected types from entt::meta
			std::map<entt::id_type, std::string> componentMap;
			for (auto&& [id, type] : entt::resolve())
			{
				const auto& info = type.info();
				auto pos = info.name().find_last_of(':') + 1;
				auto name = info.name().substr(pos);
				componentMap[id] = std::string{ name };
			}

			static std::string componentStr{};
			static std::string componentStrPrev{};
			static entt::id_type id_type{ 0 };
			static bool error{ false };

			if (ImGui::BeginCombo("Choose Component", componentStr.c_str()))
			{
				for (const auto& [id, name] : componentMap)
				{
					if (ImGui::Selectable(name.c_str(), name == componentStr))
					{
						componentStr = name;
						id_type = id;
					}
				}
				ImGui::EndCombo();
			}

			if (componentStr != componentStrPrev)
			{
				componentStrPrev = componentStr;
				error = false;
			}

			if (error)
			{
				ImGui::TextColored(ImVec4{ 1.0f, 0.0f, 0.0f, 1.0f },
					std::format("Game Object already has '{}'. Please make another selection", componentStr).c_str());
			}

			if (ImGui::Button("Ok") && !error)
			{
				if (!componentStr.empty())
				{
					for (auto&& [id, storage] : registry.storage())
					{
						if (id != id_type)
							continue;

						if (storage.contains(entity.GetEntity()))
						{
							error = true;
							F_ERROR("Entity already has component '{}'", componentStr);
							break;
						}

						break;	// The entity does not have that component yet, proceed
					}
				}

				if (!error && id_type != 0)
				{
					auto&& storage = registry.storage(id_type);
					if (!storage)
					{
						const auto addComponent = InvokeMetaFunction(id_type, "add_component_default"_hs, entity);
						if (addComponent)
						{
							EVENT_DISPATCHER().EmitEvent(AddComponentEvent{ .entity = &entity, .type = GetComponentTypeFromStr(componentStr) });

							*addComponent = false;
							ImGui::CloseCurrentPopup();
						}
						else
						{
							// This should probably fail
							F_ASSERT(addComponent && "Failed to add component");
							*addComponent = false;
							error = true;
							ImGui::CloseCurrentPopup();
						}
					}
					else
					{
						storage->push(entity.GetEntity());

						EVENT_DISPATCHER().EmitEvent(AddComponentEvent{ .entity = &entity, .type = GetComponentTypeFromStr(componentStr) });

						*addComponent = false;
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				error = false;
				*addComponent = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void SceneHierarchyDisplay::DrawGameObjectDetails()
	{
		if (!ImGui::Begin("Object Details"))
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Add Component"))
				m_AddComponent = true;

			ImGui::EndPopup();
		}

		if (m_SelectedEntity && m_AddComponent)
			AddComponent(*m_SelectedEntity, &m_AddComponent);

		if (m_SelectedEntity)
		{
			if (!m_SelectedEntity->HasComponent<UneditableComponent>())
			{
				DrawEntityComponents();
			}
			else
			{
				DrawUneditableTypes();
			}
		}

		ImGui::End();
	}

	void SceneHierarchyDisplay::DrawUneditableTypes()
	{
		if (!m_SelectedEntity)
			return;

		if (auto* pUnedit = m_SelectedEntity->TryGetComponent<UneditableComponent>())
		{
			if (pUnedit->type == EUneditableType::PlayerStart)
			{
				DrawPlayerStart();
			}
		}
	}

	void SceneHierarchyDisplay::DrawPlayerStart()
	{
		ImGui::SeparatorText("Player Start");
		ImGui::InlineLabel(ICON_FA_FLAG ICON_FA_GAMEPAD " Player Start Character: ");
		if (auto pCurrentScene = SCENE_MANAGER().GetCurrentScene())
		{
			std::string playerStartCharacter{ pCurrentScene->GetPlayerStart().GetCharacterName() };
			ImGui::SetCursorPosX(225.0f);
			ImGui::TextColored(ImVec4{ 0.7f, 0.7f, 0.7f, 1.0f }, playerStartCharacter.c_str());
		}
	}

	void SceneHierarchyDisplay::DrawEntityComponents()
	{
		auto& registry = m_SelectedEntity->GetEnttRegistry();

		for (const auto&& [id, storage] : registry.storage())
		{
			if (!storage.contains(m_SelectedEntity->GetEntity()))
				continue;

			if (id == entt::type_hash<TileComponent>::value() || id == entt::type_hash<Relationship>::value())
				continue;

			const auto drawInfo = InvokeMetaFunction(id, "DrawEntityComponentInfo"_hs, *m_SelectedEntity);

			if (drawInfo)
			{
				ImGui::Spacing();
				ImGui::PushID(id);
				if (ImGui::Button("Remove"))
				{
					storage.remove(m_SelectedEntity->GetEntity());
				}
				ImGui::PopID();
			}

			ImGui::Spacing();
			ImGui::Separator();
		}
	}

	bool SceneHierarchyDisplay::DeleteSelectedEntity()
	{
		F_ASSERT(m_SelectedEntity && "Selected Entity must be valid if trying to delete!");

		if (auto currentScene = SCENE_MANAGER().GetCurrentSceneObject())
		{
			if (m_SelectedEntity->HasComponent<UneditableComponent>())
			{
				F_ERROR("Failed to delete selected entity: Selected entity is uneditable");
				return false;
			}

			if (!currentScene->DeleteGameObjectById(m_SelectedEntity->GetEntity()))
			{
				F_ERROR("Failed to delete selected entity");
				return false;
			}

			m_SelectedEntity = nullptr;
			SCENE_MANAGER().GetToolManager().SetSelectedEntity(entt::null);
		}
		else
		{
			F_ERROR("Trying to delete an entity with no active scene");
			return false;
		}

		return true;
	}

	bool SceneHierarchyDisplay::DuplicateSelectedEntity()
	{
		F_ASSERT(m_SelectedEntity && "Selected Entity must be valid if trying to duplicate!");

		if (auto currentScene = SCENE_MANAGER().GetCurrentSceneObject())
		{
			if (!currentScene->DuplicateGameObject(m_SelectedEntity->GetEntity()))
			{
				F_ERROR("Failed to duplicate selected entity");
				return false;
			}
		}
		else
		{
			F_ERROR("Trying to duplicate an entity with no active scene");
			return false;
		}

		return true;
	}

	void SceneHierarchyDisplay::OnEntityChanged(SwitchEntityEvent& swEntEvent)
	{
		if (!swEntEvent.entity)
		{
			F_ERROR("Failed to change entity. Entity was invalid");
			return;
		}
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!currentScene)
		{
			F_ERROR("Failed to change entity. Current scene was invalid");
			return;
		}

		// Ensure that the entity is valid
		if (!currentScene->GetRegistry().IsValid(swEntEvent.entity->GetEntity()))
		{
			F_ERROR("Failed to change entity. Entity was invaild");
			return;
		}

		m_SelectedEntity = std::make_shared<Entity>(currentScene->GetRegistry(), swEntEvent.entity->GetEntity());

		F_ASSERT(m_SelectedEntity && "Entity must be valid here!");
	}

	void SceneHierarchyDisplay::OnKeyPressed(KeyEvent& keyEvent)
	{
		if (!m_WindowActive || keyEvent.type == EKeyEventType::Released)
			return;

		if (m_SelectedEntity)
		{
			auto& keyboard = INPUT_MANAGER().GetKeyboard();
			if (keyboard.IsKeyPressed(F_KEY_RCTRL) || keyboard.IsKeyPressed(F_KEY_LCTRL))
			{
				if (keyEvent.key == F_KEY_D)
				{
					DuplicateSelectedEntity();
				}
			}
			else
			{
				if (keyEvent.key == F_KEY_DELETE)
				{
					DeleteSelectedEntity();
				}
			}

		}
	}

	void SceneHierarchyDisplay::OnAddComponent(AddComponentEvent& addCompEvent)
	{
		if (addCompEvent.entity && addCompEvent.type == ComponentType::Text)
		{
			if (!addCompEvent.entity->HasComponent<UIComponent>())
			{
				addCompEvent.entity->AddComponent<UIComponent>();
			}
		}
	}

	void SceneHierarchyDisplay::OpenContext(SceneObject* currentScene)
	{
		if (!currentScene)
			return;

		if (m_SelectedEntity)
		{
			if (ImGui::BeginPopupContextWindow())
			{
				// Uneditable entities cannot be made into prefabs
				if (!m_SelectedEntity->HasComponent<UneditableComponent>())
				{
					if (ImGui::Selectable("Create Prefab"))
					{
						auto newPrefab = Feather::PrefabCreator::CreatePrefab(EPrefabType::Character, *m_SelectedEntity);
						ASSET_MANAGER().AddPrefab(m_SelectedEntity->GetName() + "_pfab", std::move(newPrefab));
					}
				}

				if (ImGui::Selectable("Delete"))
				{
					if (!DeleteSelectedEntity())
					{
						F_ERROR("Failed to delete the selected entity");
					}
				}

				if (ImGui::Selectable("Duplicate"))
				{
					if (!DuplicateSelectedEntity())
					{
						F_ERROR("Failed to duplicated the selected entity");
					}
				}

				ImGui::EndPopup();
			}
		}
		else
		{
			if (ImGui::BeginPopupContextWindow("##SceneHierarchyContext"))
			{
				if (ImGui::Selectable("Add New GameObject"))
				{
					if (!currentScene->AddGameObject())
					{
						F_ERROR("Failed to add new game object to scene '{}'", currentScene->GetSceneName());
					}
				}

				ImGui::EndPopup();
			}
		}

		if (m_WindowActive && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() &&
			(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
		{
			m_SelectedEntity = nullptr;
		}
	}

}
