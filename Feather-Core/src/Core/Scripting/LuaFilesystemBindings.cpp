#include "LuaFilesystemBindings.h"

#include "Logger/Logger.h"

namespace Feather {

	void LuaFilesystem::CreateLuaFileSystemBind(sol::state& lua)
	{
        lua.new_usertype<LuaFilesystem>(
            "Filesystem",
            sol::no_constructor,
            "file_exists",
            [](const std::string& filepath) { return std::filesystem::exists(std::filesystem::path{ filepath }); },
            "remove_file",
            [](const std::string& filepath)
            {
                std::error_code ec;
                if (!std::filesystem::exists(std::filesystem::path{ filepath }, ec))
                {
                    F_ERROR("Failed to remove file '{}': {}", filepath, ec.message());
                    return false;
                }

                if (!std::filesystem::remove(filepath, ec))
                {
                    F_ERROR("Failed to remove file '{}': {}", filepath, ec.message());
                    return false;
                }

                return true;
            },
            "create_directory",
            [](const std::string& sDirectorPath)
            {
                // Check to see if the directory already exists
                std::error_code ec;
                if (std::filesystem::is_directory(sDirectorPath, ec) || std::filesystem::exists(sDirectorPath, ec))
                {
                    F_ERROR("Directory '{}' already exists: {}", sDirectorPath, ec.message());
                    return false;
                }

                if (!std::filesystem::create_directory(sDirectorPath, ec))
                {
                    F_ERROR("Failed to create directory '{}': {}", sDirectorPath, ec.message());
                    return false;
                }

                return true;
            },
            "create_directories",
            [](const std::string& sDirectorPath)
            {
                // Check to see if the directory already exists
                std::error_code ec;
                if (std::filesystem::is_directory(sDirectorPath, ec) || std::filesystem::exists(sDirectorPath, ec))
                {
                    F_ERROR("Directory '{}' already exists: {}", sDirectorPath, ec.message());
                    return false;
                }

                if (!std::filesystem::create_directories(sDirectorPath, ec))
                {
                    F_ERROR("Failed to create directory '{}': {}", sDirectorPath, ec.message());
                    return false;
                }

                return true;
            },
            "write_file",
            [](const std::string& sFilename, const std::string& data)
            {
                // Create ofstream file
                std::ofstream outFile;
                outFile.open(sFilename);

                if (!outFile.is_open())
                {
                    F_ERROR("Could not open file '{}'. Unable to write data to file!", sFilename);
                    return false;
                }

                // Write the data to the file
                outFile << data;
                // Close the file
                outFile.close();
                // Success return true
                return true;
            },
            "read_file",
            [](const std::string& sFilename)
            {
                std::ifstream ifs(sFilename);
                std::string data{};
                // If the stream is good, read the steam into the data
                if (ifs.good())
                {
                    data.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
                }
                else
                {
                    // Set the data to an empty string
                    data = "";
                }
                return data;
            });
	}

}
