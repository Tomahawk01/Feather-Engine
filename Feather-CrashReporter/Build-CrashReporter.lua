project "Feather-CrashReporter"
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

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "NOMINMAX"
    }

    includedirs
    {
        "src",
        "%{wks.location}/vendor/SDL/include",
        "%{wks.location}/vendor/Glad/src/include",
        "%{wks.location}/vendor/ImGui/src",
    }

    libdirs
    {
        "%{wks.location}/vendor/SDL/lib"
    }

    links
    {
        "SDL2.lib",
        "Glad",
        "ImGui"
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