project "Feather-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp",
        "**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "NOMINMAX"
    }

    includedirs
    {
        "src",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/soil/include",
        "%{wks.location}/vendor/stb_image",
        "%{wks.location}/vendor/stb_truetype",
        "%{wks.location}/vendor/glm",
        "%{wks.location}/vendor/entt/include",
        "%{wks.location}/vendor/lua_53/include",
        "%{wks.location}/vendor/sol3",
        "%{wks.location}/vendor/box2d/include",
        "%{wks.location}/vendor/tinyfiledialogs/include"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib",
        "%{wks.location}/vendor/lua_53/lib",
        "%{wks.location}/vendor/box2d/lib",
        "%{wks.location}/vendor/tinyfiledialogs/lib"
    }

    links
    {
        "Glad",
        "SDL2.lib",
        "SDL2_mixer.lib",
        "lua53.lib",
        "tinyfiledialogs64.lib",
        "Box2D",
        "SOIL"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"