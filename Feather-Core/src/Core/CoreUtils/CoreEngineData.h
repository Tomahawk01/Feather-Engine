#pragma once

#include <chrono>

#define CORE_GLOBALS() Feather::CoreEngineData::GetInstance()

namespace Feather {

	class CoreEngineData
	{
	public:
		static CoreEngineData& GetInstance();
		void UpdateDeltaTime();
		void SetWindowWidth(int windowWidth);
		void SetWindowHeight(int windowHeight);

		const float MetersToPixels() const;
		const float PixelsToMeters() const;

		inline const float GetDeltaTime() const { return m_DeltaTime; }
		inline const int WindowWidth() const { return m_WindowWidth; }
		inline const int WindowHeight() const { return m_WindowHeight; }

		inline void EnableColliderRender() { m_RenderColliders = true; }
		inline void DisableColliderRender() { m_RenderColliders = false; }
		inline const bool RenderCollidersEnabled() { return m_RenderColliders; }

		inline const float ScaledWidth() const { return m_ScaledWidth; }
		inline const float ScaledHeight() const { return m_ScaledHeight; }

		inline const int32_t GetVelocityIterations() const { return m_VelocityIterations; }
		inline const int32_t GetPositionIterations() const { return m_PositionIterations; }

		inline void EnablePhysics() { m_PhysicsEnabled = true; }
		inline void DisablePhysics() { m_PhysicsEnabled = false; }
		inline void PausePhysics() { m_PhysicsPaused = true; }
		inline void UnPausePhysics() { m_PhysicsPaused = false; }
		inline const bool IsPhysicsEnabled() const { return m_PhysicsEnabled; }
		inline const bool IsPhysicsPaused() const { return m_PhysicsPaused; }

		static void RegisterMetaFunctions();

	private:
		CoreEngineData();
		~CoreEngineData() = default;
		CoreEngineData(const CoreEngineData&) = delete;
		CoreEngineData& operator=(const CoreEngineData&) = delete;

	private:
		float m_DeltaTime, m_ScaledWidth, m_ScaledHeight;
		std::chrono::steady_clock::time_point m_LastUpdate;
		int m_WindowWidth, m_WindowHeight;
		int32_t m_VelocityIterations, m_PositionIterations;

		bool m_PhysicsEnabled, m_PhysicsPaused, m_RenderColliders;
	};

}
