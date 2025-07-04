#pragma once
#include <glm/glm.hpp>

namespace Feather {

	struct Canvas;
	class Camera2D;
	class Registry;
	class SceneObject;

	class AbstractTool
	{
	public:
		AbstractTool();
		virtual ~AbstractTool() = default;

		// NOTE: If overrided, ensure to call the parent update. This will make sure the mouse world coords are updated accordingly.
		virtual void Update(Canvas& canvas);
		bool SetupTool(SceneObject* sceneObject, Camera2D* camera);

		inline void SetRelativeCoords(const glm::vec2& relativeCoords) { m_GUIRelativeCoords = relativeCoords; }
		inline void SetCursorCoords(const glm::vec2& cursorCoords) { m_GUICursorCoords = cursorCoords; }
		inline void SetWindowPos(const glm::vec2& windowPos) { m_WindowPos = windowPos; }
		inline void SetWindowSize(const glm::vec2& windowSize) { m_WindowSize = windowSize; }

		inline const glm::vec2& GetMouseScreenCoords() const { return m_MouseScreenCoords; }
		inline const glm::vec2& GetMouseWorldCoords() const { return m_MouseWorldCoords; }
		inline const glm::vec2& GetGridCoords() const { return m_GridCoords; }
		inline const glm::vec2& GetWindowSize() const { return m_WindowSize; }

		inline void Activate() { m_Activated = true; }
		inline void Deactivate() { m_Activated = false; }
		inline void SetOverTilemapWindow(bool isOverWindow) { m_OverTileMapWindow = isOverWindow; }

		inline const bool IsActivated() const { return m_Activated; }
		inline const bool OutOfBounds() const { return m_OutOfBounds; }
		inline const bool IsOverTilemapWindow() const { return m_OverTileMapWindow; }

	private:
		void UpdateMouseWorldCoords();
		void CheckOutOfBounds(const Canvas& canvas);

	private:
		glm::vec2 m_MouseScreenCoords, m_MouseWorldCoords;
		glm::vec2 m_GUICursorCoords, m_GUIRelativeCoords;
		glm::vec2 m_WindowPos, m_WindowSize;

		bool m_Activated;
		bool m_OutOfBounds;
		bool m_OverTileMapWindow;

	protected:
		enum class MouseButton
		{
			UNKNOWN = 0,
			LEFT,
			MIDDLE,
			RIGHT,

			LAST
		};

		bool MouseButtonJustPressed(MouseButton button);
		bool MouseButtonJustReleased(MouseButton button);
		bool MouseButtonPressed(MouseButton button);
		bool MouseMoving();

		/*
		* @brief Used for different tools to make adjustments to the mouse position if necessary.
		*/
		virtual void ExamineMousePosition() = 0;
		virtual void ResetSelectedEntity() {}

		inline void SetMouseWorldCoords(const glm::vec2& newCoords) { m_MouseWorldCoords = newCoords; }

	protected:
		Registry* m_Registry{ nullptr };
		Camera2D* m_Camera{ nullptr };
		SceneObject* m_CurrentScene{ nullptr };
		glm::vec2 m_GridCoords;
	};

}
