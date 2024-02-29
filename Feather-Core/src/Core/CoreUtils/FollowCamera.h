#pragma once

#include "Renderer/Core/Camera2D.h"
#include "CoreEngineData.h"
#include "Core/ECS/Entity.h"

namespace Feather {

	struct FollowCamParams
	{
		float minX{ 0.0f }, minY{ 0.0f }, scale{ 1.0f }, springback{ 1.0f };
		float maxX{ static_cast<float>(CoreEngineData::GetInstance().WindowWidth()) };
		float maxY{ static_cast<float>(CoreEngineData::GetInstance().WindowHeight()) };
	};

	class FollowCamera
	{
	public:
		FollowCamera(Camera2D& camera, const Entity& entity, const FollowCamParams& params);
		~FollowCamera() = default;
		
		void Update();

		inline void SetCameraParameters(const FollowCamParams& params) { m_Params = params; }
		inline void SetEntity(const Entity& entity) { m_Entity = entity; }
		inline void SetSpringback(float springback) { m_Params.springback = springback; }
		inline const FollowCamParams& GetParams() const { return m_Params; }

		static void CreateLuaFollowCamera(sol::state& lua, Registry& registry);

	private:
		Camera2D& m_Camera;
		Entity m_Entity;
		FollowCamParams m_Params;
	};

}
