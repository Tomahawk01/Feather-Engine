project "Feather-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp",
        "**.inl",
        "%{wks.location}/vendor/stb_image/**.cpp",
        "%{wks.location}/vendor/stb_image/**.h"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "src",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/stb_image",
        "%{wks.location}/vendor/glm",
        "%{wks.location}/vendor/spdlog-1.12.0/include",
        "%{wks.location}/vendor/entt/include",
        "%{wks.location}/vendor/lua_53/include",
        "%{wks.location}/vendor/sol3"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib",
        "%{wks.location}/vendor/lua_53/lib"
    }

    links
    {
        "Glad",
        "SDL2.lib",
        "lua53.lib"
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
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"