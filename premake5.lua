workspace "Mango"
    architecture "x64"
    configurations {
        "Debug",
        "Release"
    }

    startproject "MangoEditor"

Includes = {};
Includes["spdlog"] = "Mango/vendor/spdlog/include"
Includes["ImGui"] = "Mango/vendor/ImGui"
Includes["stb_image"] = "Mango/vendor/stb_image"
Includes["json"] = "Mango/vendor/json/include"

TargetDir = "bin/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"
ObjDir = "bin-int/%{prj.name}/%{cfg.buildcfg}-%{cfg.architecture}"

project "ImGui"
    kind "StaticLib"
    location "Mango/vendor/ImGui"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
    systemversion "latest"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "%{prj.location}/*.h",
        "%{prj.location}/*.cpp",
    }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        defines "NDEBUG"
        optimize "On"

project "Mango"
    kind "StaticLib"
    location "%{prj.name}"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    pchheader "mgpch.h"
    pchsource "%{prj.name}/src/mgpch.cpp"

    includedirs {
        "%{prj.name}/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.stb_image}",
        "%{Includes.json}",
        "Mango/vendor/assimp/include"
    }

    links {
        "dxguid.lib",
        "ImGui"
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

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/Shaders/**.hlsl",
    }

    libdirs {
        "Mango/vendor/assimp/bin"
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib",
        "assimp.lib"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.stb_image}"
    }

    filter "files:**.hlsl"
		shaderobjectfileoutput("assets/shaders/%{file.basename}"..".cso")
		shadermodel "5.0"

	filter "files:**_ps.hlsl"
		shadertype "Pixel"

	filter "files:**_vs.hlsl"
        shadertype "Vertex"
        
    filter "files:**_gs.hlsl"
        shadertype "Geometry"

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"

project "MangoEditor"
    kind "WindowedApp"
    location "%{prj.name}"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/Shaders/**.hlsl",
        "%{prj.name}/vendor/ImGuizmo/*.h",
        "%{prj.name}/vendor/ImGuizmo/*.cpp",
    }
    
    libdirs {
        "Mango/vendor/assimp/bin"
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib",
        "assimp.lib"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.stb_image}",
        "%{prj.name}/vendor/ImGuizmo",
    }

    filter "files:**.hlsl"
		shaderobjectfileoutput("assets/shaders/%{file.basename}"..".cso")
		shadermodel "5.0"

	filter "files:**_ps.hlsl"
		shadertype "Pixel"

	filter "files:**_vs.hlsl"
        shadertype "Vertex"
    
    filter "files:**_gs.hlsl"
        shadertype "Geometry"

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"

project "Tests"
    kind "ConsoleApp"
    location "%{prj.name}"
    language "C++"
    cppdialect "C++17"
	staticruntime "On"
	systemversion "latest"

    targetdir "%{TargetDir}"
    objdir "%{ObjDir}"

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/Shaders/**.hlsl"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.stb_image}",
        "Tests/vendor/catch2"
    }

    libdirs {
        "Mango/vendor/assimp/bin"
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib",
        "assimp.lib"
    }

    filter "files:**.hlsl"
		shaderobjectfileoutput("assets/shaders/%{file.basename}"..".cso")
		shadermodel "5.0"

	filter "files:**_ps.hlsl"
		shadertype "Pixel"

	filter "files:**_vs.hlsl"
        shadertype "Vertex"

    filter "files:**_gs.hlsl"
        shadertype "Geometry"    

    filter "configurations:Debug"
        defines "MG_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines {
            "MG_RELEASE",
            "NDEBUG"
        }
        optimize "On"