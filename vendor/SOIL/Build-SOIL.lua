project "SOIL"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    files
    {
        "**.h",
        "**.c"
    }

    includedirs
    {
        "include"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"