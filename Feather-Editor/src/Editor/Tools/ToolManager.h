#pragma once
#include <map>
#include <memory>

namespace Feather {

	enum class ToolType;
	class TileTool;
	struct Canvas;

	class ToolManager
	{
	public:
		ToolManager();
		~ToolManager() = default;

		void Update(Canvas& canvas);

		/*
		* @brief Activates the tool based on the passed in tool type. This will also deactivate all other tools
		* @param Tool type to set active.
		*/
		void SetToolActive(ToolType toolType);

		/*
		* @brief Gets the current activated tool
		* @return Returns a pointer to a TileTool if activated, else returns nullptr
		*/
		TileTool* GetActiveTool();

	private:
		std::map<ToolType, std::unique_ptr<TileTool>> m_mapTools;
	};

}
