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
    include "vendor/box2d/Build-Box2D.lua"
group ""

include "Feather-Core/Build-Core.lua"
include "Feather-Editor/Build-Editor.lua"