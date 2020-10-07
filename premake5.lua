workspace "Mango"
    architecture "x64"
    configurations {
        "Debug",
        "Release"
    }

    startproject "Sandbox"

Includes = {};
Includes["spdlog"] = "Mango/vendor/spdlog/include"

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
        "%{prj.name}/src",
        "%{Includes.spdlog}"
    }

    links {
        "dxguid.lib",
        "dxgi.lib"
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
        "%{prj.name}/src/Shaders/**.hlsl"
    }

    links {
        "Mango",
        "d3d11.lib",
        "d3dcompiler.lib"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}"
    }

    filter "files:**.hlsl"
		shaderobjectfileoutput("assets/shaders/%{file.basename}"..".cso")
		shadermodel "4.0"

	filter "files:**_ps.hlsl"
		shadertype "Pixel"

	filter "files:**_vs.hlsl"
        shadertype "Vertex"
        

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"