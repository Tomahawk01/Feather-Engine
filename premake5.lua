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
group ""

include "Feather-Core/Build-Core.lua"
include "Feather-Editor/Build-Editor.lua"