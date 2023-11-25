project "Feather-Core"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files
    {
        "**.h",
        "**.cpp"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "src",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/SOIL/include",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/glm"
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib"
    }

    links
    {
        "Glad",
        "SOIL",
        "SDL2.lib"
    }

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"