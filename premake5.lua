workspace "Feather-Engine"
    architecture "x64"
    startproject "Feather-Editor"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    filter "system:windows"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "vendor/Glad/Build-Glad.lua"
    include "vendor/SOIL/Build-SOIL.lua"
group ""

include "Feather-Editor/Build-Editor.lua"
include "Feather-Utilities/Build-Utilities.lua"
include "Feather-Window/Build-Window.lua"