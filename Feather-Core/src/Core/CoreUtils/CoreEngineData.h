#pragma once

#define CORE_GLOBALS() Feather::CoreEngineData::GetInstance()

namespace Feather {

	enum class GameType
	{
		TopDown,
		Platformer,
		Fighting,
		Puzzle,
		Rougelike,

		NoType
	};

	class CoreEngineData
	{
	public:
		static CoreEngineData& GetInstance();
		void UpdateDeltaTime();
		void SetWindowWidth(int windowWidth);
		void SetWindowHeight(int windowHeight);

		const float MetersToPixels() const;
		const float PixelsToMeters() const;

		void SetScaledWidth(float newWidth);
		void SetScaledHeight(float newHeight);

		inline double GetDeltaTime() const { return m_DeltaTime; }
		inline int WindowWidth() const { return m_WindowWidth; }
		inline int WindowHeight() const { return m_WindowHeight; }

		inline void EnableColliderRender() { m_RenderColliders = true; }
		inline void DisableColliderRender() { m_RenderColliders = false; }
		inline bool RenderCollidersEnabled() const { return m_RenderColliders; }

		inline void EnableAnimationRender() { m_RenderAnimations = true; }
		inline void DisableAnimationRender() { m_RenderAnimations = false; }
		inline bool AnimationRenderEnabled() const { return m_RenderAnimations; }

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

		inline void SetGameType(GameType type) { m_GameType = type; }
		inline GameType GetGameType() const { return m_GameType; }

		static std::string GetGameTypeStr(GameType eType);
		static GameType GetGameTypeFromStr(const std::string& sType);

		static const std::unordered_map<GameType, std::string>& GetGameTypesMap();

		static void RegisterMetaFunctions();

	private:
		CoreEngineData();
		~CoreEngineData() = default;
		CoreEngineData(const CoreEngineData&) = delete;
		CoreEngineData& operator=(const CoreEngineData&) = delete;

	private:
		double m_DeltaTime;
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
		bool m_RenderAnimations;

		std::string m_ProjectPath;

		GameType m_GameType{ GameType::NoType };
	};

}
