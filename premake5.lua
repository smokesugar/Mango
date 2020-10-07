workspace "Mango"
    architecture "x64"
    configurations {
        "Debug",
        "Release"
    }

    startproject "Sandbox"

project "Mango"
    kind "StaticLib"
    location "%{prj.name}"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"

    targetdir "bin/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"
    objdir "bin-int/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    pchheader "mgpch.h"
    pchsource "%{prj.name}/src/mgpch.cpp"

    includedirs {
        "%{prj.name}/src"
    }

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"

project "Sandbox"
    kind "WindowedApp"
    location "%{prj.name}"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"

    targetdir "bin/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"
    objdir "bin-int/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    links {
        "Mango"
    }

    includedirs {
        "Mango/src"
    }

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"