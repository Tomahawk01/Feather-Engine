project "Feather-Runtime"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    pchheader "RuntimePCH.h"
    pchsource "src/RuntimePCH.cpp"
    forceincludes { "RuntimePCH.h" }

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
        "%{wks.location}/Feather-Core/src",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/soil/include",
        "%{wks.location}/vendor/stb_image",
        "%{wks.location}/vendor/stb_truetype",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/glm",
        "%{wks.location}/vendor/entt/include",
        "%{wks.location}/vendor/lua_5.4.2/include",
        "%{wks.location}/vendor/sol3",
        "%{wks.location}/vendor/box2d/include",
        "%{wks.location}/vendor/libzippp/include"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib",
        "%{wks.location}/vendor/lua_5.4.2/lib",
        "%{wks.location}/vendor/libzippp/lib"
    }

    links
    {
        "Feather-Core",
        "SDL2.lib",
        "SDL2_mixer.lib",
        "SDL2_image.lib",
        "opengl32.lib",
        "lua54.lib",
        "Glad",
        "Box2D",
        "SOIL",
        "libzippp.lib"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }

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