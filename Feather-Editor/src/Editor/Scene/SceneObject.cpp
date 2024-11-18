#include "SceneObject.h"
#include "Utils/FeatherUtilities.h"
#include "Core/ECS/MetaUtilities.h"

#include <format>

using namespace entt::literals;

namespace Feather {

	SceneObject::SceneObject(const std::string& sceneName)
		: m_Registry{}, m_RuntimeRegistry{}, m_SceneName{ sceneName }, m_Canvas{}, m_CurrentLayer{ 0 }
	{}

	void SceneObject::CopySceneToRuntime()
	{
		auto& registryToCopy = m_Registry.GetRegistry();

		for (auto entityToCopy : registryToCopy.view<entt::entity>(entt::exclude<ScriptComponent>))
		{
			entt::entity newEntity = m_RuntimeRegistry.CreateEntity();

			// Copy components of the entity to the new entity
			for (auto&& [id, storage] : registryToCopy.storage())
			{
				if (!storage.contains(entityToCopy))
					continue;

				InvokeMetaFunction(id, "copy_component"_hs, Entity{ m_Registry, entityToCopy }, Entity{ m_RuntimeRegistry, newEntity });
			}
		}
	}

	void SceneObject::ClearRuntimeScene()
	{
		m_RuntimeRegistry.ClearRegistry();
	}

	void SceneObject::AddNewLayer()
	{
		m_LayerParams.emplace_back(SpriteLayerParams{ .layerName = std::format("NewLayer_{}", m_CurrentLayer++) });
	}

	bool SceneObject::CheckLayerName(const std::string& layerName)
	{
		return CheckContainsValue(m_LayerParams,
			[&](SpriteLayerParams& spriteLayer) { return spriteLayer.layerName == layerName; }
		);
	}

}