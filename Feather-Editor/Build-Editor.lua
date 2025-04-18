project "Feather-Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp",
        "**.inl"
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
        "%{wks.location}/vendor/lua_53/include",
        "%{wks.location}/vendor/sol3",
        "%{wks.location}/vendor/box2d/include",
        "%{wks.location}/vendor/ImGui/src",
        "%{wks.location}/vendor/rapidjson/include"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib",
        "%{wks.location}/vendor/lua_53/lib"
    }

    links
    {
        "Feather-Core",
        "SDL2.lib",
        "SDL2_mixer.lib",
        "opengl32.lib",
        "lua53.lib",
        "Glad",
        "Box2D",
        "ImGui",
        "SOIL"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "WINDOWS" }
        files { "%{wks.location}/resources/feather.rc", "**.ico" }
        vpaths { ["resources/*"] = { "*.rc", "**.ico" } }

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