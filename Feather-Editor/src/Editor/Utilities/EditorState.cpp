#include "EditorState.h"

#include "Logger/Logger.h"
#include "FileSystem/Serializers/JSONSerializer.h"
#include "Core/CoreUtils/ProjectInfo.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace Feather {

	bool EditorState::Save(ProjectInfo& projectInfo)
	{
		const auto optEditorConfigFolder = projectInfo.TryGetFolderPath(EProjectFolderType::EditorConfig);
		if (!optEditorConfigFolder)
		{
			F_ERROR("Failed to save editor state. Editor config path is invalid");
			return false;
		}

		if (!fs::exists(*optEditorConfigFolder))
		{
			F_ERROR("Failed to save editor state at path '{}'", optEditorConfigFolder->string());
			return false;
		}

		std::unique_ptr<JSONSerializer> serializer{ nullptr };
		fs::path editorStatePath = *optEditorConfigFolder / "editor_state.config";

		try
		{
			serializer = std::make_unique<JSONSerializer>(editorStatePath.string());
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save editor state to file '{}': {}", editorStatePath.string(), ex.what());
			return false;
		}

		serializer->StartDocument();
		serializer->StartNewObject("warnings");
		serializer->AddKeyValuePair("warning", std::string{ "This file is engine generated" })
			.AddKeyValuePair("warning", std::string{ "DO NOT CHANGE unless you know what you are doing" })
			.EndObject(); // Warnings

		serializer->StartNewObject("editor_state")
			.AddKeyValuePair("active_displays", activeDisplays)
			.AddKeyValuePair("gridSnap", enableGridsnap)
			.AddKeyValuePair("showAnimations", showAnimations)
			.AddKeyValuePair("showCollisions", showCollisions)
			.EndObject(); // EditorState

		return serializer->EndDocument();
	}

	bool EditorState::Load(ProjectInfo& projectInfo)
	{
		const auto optEditorConfigFolder = projectInfo.TryGetFolderPath(EProjectFolderType::EditorConfig);
		if (!optEditorConfigFolder)
		{
			F_ERROR("Failed to load editor state. Editor config path is invalid");
			return false;
		}

		fs::path editorStatePath = *optEditorConfigFolder / "editor_state.config";
		if (!fs::exists(editorStatePath))
		{
			F_ERROR("Failed to load editor state at path '{}'", editorStatePath.string());
			return false;
		}

		std::ifstream editorStateFile;
		editorStateFile.open(editorStatePath.string());

		if (!editorStateFile.is_open())
		{
			F_ERROR("Failed to open project file '{}'", editorStatePath.string());
			return false;
		}

		std::stringstream ss;
		ss << editorStateFile.rdbuf();
		std::string contents = ss.str();
		rapidjson::StringStream jsonStr{ contents.c_str() };

		rapidjson::Document doc;
		doc.ParseStream(jsonStr);

		if (doc.HasParseError() || !doc.IsObject())
		{
			F_ERROR("Failed to load editor state. File '{}' is not valid JSON, Error: {}, Offset: {}", editorStatePath.string(), rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset());
			return false;
		}

		// Get the project data
		if (!doc.HasMember("editor_state"))
		{
			F_ERROR("Failed to load project file '{}': Expecting \"editor_state\" member in project file", editorStatePath.string());
			return false;
		}

		const rapidjson::Value& editorState = doc["editor_state"];

		activeDisplays = editorState["active_displays"].GetUint64();
		showAnimations = editorState["showAnimations"].GetBool();
		showCollisions = editorState["showCollisions"].GetBool();
		enableGridsnap = editorState["gridSnap"].GetBool();

		return true;
	}

	bool EditorState::ImportState(const std::string& sImportedStateFile, ProjectInfo& projectInfo)
	{
		return false;
	}

	bool EditorState::CreateEditorStateFile(ProjectInfo& projectInfo)
	{
		const auto optEditorConfigFolder = projectInfo.TryGetFolderPath(EProjectFolderType::EditorConfig);
		if (!optEditorConfigFolder)
		{
			F_ERROR("Failed to save editor state. Editor config path is invalid");
			return false;
		}

		if (!fs::exists(*optEditorConfigFolder))
		{
			F_ERROR("Failed to save editor state at path '{}'", optEditorConfigFolder->string());
			return false;
		}

		std::unique_ptr<JSONSerializer> serializer{ nullptr };
		fs::path editorStatePath = *optEditorConfigFolder / "editor_state.config";

		try
		{
			serializer = std::make_unique<JSONSerializer>(editorStatePath.string());
		}
		catch (const std::exception& ex)
		{
			F_ERROR("Failed to save editor state to file '{}': {}", editorStatePath.string(), ex.what());
			return false;
		}

		serializer->StartDocument();
		serializer->StartNewObject("warnings");
		serializer->AddKeyValuePair("warning", std::string{ "This file is engine generated" })
			.AddKeyValuePair("warning", std::string{ "DO NOT CHANGE unless you know what you are doing" })
			.EndObject(); // Warnings

		serializer->StartNewObject("editor_state")
			.AddKeyValuePair(
				"active_displays",
				static_cast<uint64_t>(EDisplay::Hierarchy) | static_cast<uint64_t>(EDisplay::AssetBrowser) |
				static_cast<uint64_t>(EDisplay::Console) | static_cast<uint64_t>(EDisplay::TilemapView))
			.AddKeyValuePair("gridSnap", true)
			.AddKeyValuePair("showAnimations", false)
			.AddKeyValuePair("showCollisions", false)
			.EndObject(); // EditorState

		return serializer->EndDocument();
	}

}
