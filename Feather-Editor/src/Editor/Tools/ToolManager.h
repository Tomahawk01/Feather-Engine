#pragma once
#include <map>
#include <memory>
#include <string>

namespace Feather {

	enum class ToolType;
	enum class GizmoType;
	class TileTool;
	class SceneObject;
	class Camera2D;
	struct Canvas;

	class ToolManager
	{
	public:
		ToolManager();
		~ToolManager() = default;

		void Update(Canvas& canvas);

		/*
		* @brief Activates the tool based on the passed in tool type. This will also deactivate all other tools and gizmos
		* @param Tool type to set active.
		*/
		void SetToolActive(ToolType toolType);

		/*
		* @brief Sets the passed in tileset for all tools
		*/
		void SetToolsCurrentTileset(const std::string& tileset);

		void SetTileToolStartCoords(int x, int y);

		/*
		* @brief Activates the gizmo based on the passed in gizmo type. This will also deactivate all other tools and gizmos
		* @param Gizmo type to set active.
		*/
		void SetGizmoActive(GizmoType gizmoType);

		/*
		* @brief Gets the current activated tool
		* @return Returns a pointer to a TileTool if activated, else returns nullptr
		*/
		TileTool* GetActiveTool();

		inline ToolType GetActiveToolType() const { return m_ActiveToolType; }
		inline GizmoType GetActiveGizmoType() const { return m_ActiveGizmoType; }

		bool SetupTools(SceneObject* sceneObject, Camera2D* camera);

		void EnableGridSnap(bool enable);

	private:
		std::map<ToolType, std::unique_ptr<TileTool>> m_mapTools;

		ToolType m_ActiveToolType;
		GizmoType m_ActiveGizmoType;
	};

}
