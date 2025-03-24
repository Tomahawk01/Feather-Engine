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

		inline float GetDeltaTime() const { return m_DeltaTime; }
		inline int WindowWidth() const { return m_WindowWidth; }
		inline int WindowHeight() const { return m_WindowHeight; }

		inline void EnableColliderRender() { m_RenderColliders = true; }
		inline void DisableColliderRender() { m_RenderColliders = false; }
		inline bool RenderCollidersEnabled() const { return m_RenderColliders; }

		inline float ScaledWidth() const { return m_ScaledWidth; }
		inline float ScaledHeight() const { return m_ScaledHeight; }

		inline int32_t GetVelocityIterations() const { return m_VelocityIterations; }
		inline int32_t GetPositionIterations() const { return m_PositionIterations; }

		inline void SetVelocityIterations(int32_t velocityIterations) { m_VelocityIterations = velocityIterations; }
		inline void SetPositionIterations(int32_t positionIterations) { m_PositionIterations = positionIterations; }

		inline float GetGravity() const { return m_Gravity; }
		inline void SetGravity(float gravity) { m_Gravity = gravity; }

		inline void EnablePhysics() { m_PhysicsEnabled = true; }
		inline void DisablePhysics() { m_PhysicsEnabled = false; }
		inline void PausePhysics() { m_PhysicsPaused = true; }
		inline void UnPausePhysics() { m_PhysicsPaused = false; }
		inline const bool IsPhysicsEnabled() const { return m_PhysicsEnabled; }
		inline const bool IsPhysicsPaused() const { return m_PhysicsPaused; }

		inline const std::string& GetProjectPath() const { return m_ProjectPath; }
		inline void SetProjectPath(const std::string& path) { m_ProjectPath = path; }

		static void RegisterMetaFunctions();

	private:
		CoreEngineData();
		~CoreEngineData() = default;
		CoreEngineData(const CoreEngineData&) = delete;
		CoreEngineData& operator=(const CoreEngineData&) = delete;

	private:
		float m_DeltaTime;
		float m_ScaledWidth;
		float m_ScaledHeight;
		float m_Gravity;
		std::chrono::steady_clock::time_point m_LastUpdate;
		int m_WindowWidth;
		int m_WindowHeight;
		int32_t m_VelocityIterations;
		int32_t m_PositionIterations;

		bool m_PhysicsEnabled;
		bool m_PhysicsPaused;
		bool m_RenderColliders;

		std::string m_ProjectPath;
	};

}
