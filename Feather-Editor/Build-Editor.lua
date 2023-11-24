project "Feather-Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Feather-Utilities",
        "%{wks.location}/Feather-Window",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/SOIL/include",
        "%{wks.location}/vendor/glm"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib"
    }

    links
    {
        "Feather-Utilities",
        "Feather-Window",
        "Glad",
        "SOIL",
        "SDL2.lib",
        "opengl32.lib"
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
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"