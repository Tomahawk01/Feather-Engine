project "SOIL"
	kind "StaticLib"
	language "C++"
	cppdialect "C++11"
	staticruntime "off"

    files
	{
		"src/**.c",
		"include/**.h"
	}

	includedirs
	{
		"include",
		"src",
		"%{wks.location}/vendor/Glad/src/include",
	}

	links
	{
		"Glad"
	}

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		symbols "Off"