#pragma once

#include "Core/ECS/Entity.h"

#include <glm/glm.hpp>

namespace Feather {

	class Prefab;
	class Character;
	class Scene;

	class PlayerStart
	{
	public:
		PlayerStart(Registry& registry, Scene& sceneRef);

		void CreatePlayer(Registry& registry);
		std::string GetCharacterName();
		void SetCharacter(const Prefab& prefab);

		glm::vec2 GetPosition();
		void SetPosition(const glm::vec2& position);

		/*
		 * @brief Loads the character name and sets the prefab to be loaded.
		 * The prefabs might not be loaded when loading the scenes that owns this player start
		 */
		void Load(const std::string& prefabName);

		inline bool IsCharacterSet() const { return m_CharacterPrefab != nullptr; }

	private:
		Scene& m_SceneRef;
		Entity m_VisualEntity;
		std::shared_ptr<Prefab> m_CharacterPrefab;
		std::string m_CharacterName;
		bool m_CharacterLoaded;

		// TODO: Determine if this is needed
		std::shared_ptr<Character> m_Character;
	};

}
