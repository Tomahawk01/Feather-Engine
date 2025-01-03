#pragma once

#include "Renderer/Core/Camera2D.h"
#include "CoreEngineData.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	struct FollowCamParams
	{
		float minX{ 0.0f };
		float minY{ 0.0f };
		float scale{ 1.0f };
		float springback{ 1.0f };
		float maxX{ static_cast<float>(CoreEngineData::GetInstance().WindowWidth()) };
		float maxY{ static_cast<float>(CoreEngineData::GetInstance().WindowHeight()) };
	};

	class FollowCamera
	{
	public:
		FollowCamera(Camera2D& camera, const Entity& entity, const FollowCamParams& params);
		~FollowCamera() = default;
		
		void Update();
		void SetSpringback(float springback);
		void SetCameraParameters(const FollowCamParams& params);

		inline void SetEntity(const Entity& entity) { m_Entity = entity; }
		inline const FollowCamParams& GetParams() const { return m_Params; }

		static void CreateLuaFollowCamera(sol::state& lua, Registry& registry);

	private:
		Camera2D& m_Camera;
		Entity m_Entity;
		FollowCamParams m_Params;
	};

}
