project "Feather-Renderer"
    kind "StaticLib"
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
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/SOIL/include",
        "%{wks.location}/vendor/glm"
    }

    links
    {
        "Glad",
        "SOIL"
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