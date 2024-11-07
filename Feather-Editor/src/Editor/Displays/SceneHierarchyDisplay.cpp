#include "SceneHierarchyDisplay.h"
#include "Core/ECS/MainRegistry.h"
#include "Core/ECS/Entity.h"
#include "Core/ECS/Components/AllComponents.h"
#include "Core/ECS/MetaUtilities.h"

#include "../Scene/SceneManager.h"
#include "../Scene/SceneObject.h"

#include <imgui.h>

#include <format>

using namespace entt::literals;

namespace Feather {

	auto create_entity = [&](SceneObject& currentScene) {
		Entity newEntity{ currentScene.GetRegistry(), "GameObject", "" };
		newEntity.AddComponent<TransformComponent>();
	};

	SceneHierarchyDisplay::SceneHierarchyDisplay()
	{}

	SceneHierarchyDisplay::~SceneHierarchyDisplay()
	{}

	void SceneHierarchyDisplay::Update()
	{
	}

	void SceneHierarchyDisplay::Draw()
	{
		auto currentScene = SCENE_MANAGER().GetCurrentScene();
		if (!ImGui::Begin("Scene Hierarchy") || !currentScene)
		{
			ImGui::End();
			return;
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Add new GameObject"))
				create_entity(*currentScene);

			ImGui::EndPopup();
		}

		auto& registry = currentScene->GetRegistry();
		auto sceneEntities = registry.GetRegistry().view<entt::entity>(entt::exclude<TileComponent, ScriptComponent>);

		for (auto entity : sceneEntities)
		{
			Entity ent{ registry, entity };
			if (OpenTreeNode(ent))
				ImGui::TreePop();
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

		if (m_SelectedEntity && m_SelectedEntity->GetEntity() == entity.GetEntity())
			nodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool treeNodeOpen{ false };
		treeNodeOpen = ImGui::TreeNodeEx(name.c_str(), nodeFlags);

		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = std::make_shared<Entity>(SCENE_MANAGER().GetCurrentScene()->GetRegistry(), entity.GetEntity());
		}

		ImGui::PopID();

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
			auto& registry = entity.GetRegistry();

			// Get all of the reflected types from entt::meta
			std::map<entt::id_type, std::string> componentMap;
			for (auto&& [id, type] : entt::resolve())
			{
				const auto& info = type.info();
				auto pos = info.name().find_last_of(':') + 1;
				auto name = info.name().substr(pos);
				componentMap[id] = std::string{ name };
			}

			static std::string componentStr{ "" };
			static std::string componentStrPrev{ "" };
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

			if (ImGui::Button("Ok"))
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

				if (error)
				{
					*addComponent = false;
					ImGui::CloseCurrentPopup();
				}
				else
				{
					auto&& storage = registry.storage(id_type);
					if (!storage)
					{
						const auto addComponent = InvokeMetaFunction(id_type, "add_component_default"_hs, entity);
						if (addComponent)
						{
							*addComponent = false;
							ImGui::CloseCurrentPopup();
						}
						else
						{
							F_ASSERT(addComponent && "Failed to add component");
							*addComponent = false;
							error = true;
							ImGui::CloseCurrentPopup();
						}
					}
					else
					{
						storage->push(entity.GetEntity());
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
			DrawEntityComponents();

		ImGui::End();
	}

	void SceneHierarchyDisplay::DrawEntityComponents()
	{
		if (!m_SelectedEntity)
			return;

		auto& registry = m_SelectedEntity->GetRegistry();

		for (const auto&& [id, storage] : registry.storage())
		{
			if (!storage.contains(m_SelectedEntity->GetEntity()))
				continue;

			if (id == entt::type_hash<TileComponent>::value())
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

}
