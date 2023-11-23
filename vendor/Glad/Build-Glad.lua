project "Glad"
    kind "StaticLib"
    language "C"
    staticruntime "off"

    files
    {
        "src/include/glad/glad.h",
        "src/include/KHR/khrplatform.h",
        "src/glad.c"
    }

    includedirs
    {
        "src/include"
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