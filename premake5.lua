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

    filter { "files:%{wks.location}/vendor/**.cpp", "files:%{wks.location}/vendor/**.c" }
        flags { "NoPCH" }
    filter {}

    filter "system:windows"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "vendor/Glad/Build-Glad.lua"
    include "vendor/box2d/Build-Box2D.lua"
    include "vendor/ImGui/Build-ImGui.lua"
    include "vendor/soil/Build-SOIL.lua"
group ""

include "Feather-Core/Build-Core.lua"
include "Feather-Editor/Build-Editor.lua"
include "Feather-CrashReporter/Build-CrashReporter.lua"
include "Feather-Runtime/Build-Runtime.lua"