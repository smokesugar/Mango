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
Includes["Lua"] = "Mango/vendor/Lua/include"

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
        "%{Includes.Lua}",
        "Mango/vendor/assimp/include"
    }

    libdirs {
        "Mango/vendor/assimp/bin",
        "Mango/vendor/Lua/bin"
    }

    links {
        "dxguid.lib",
        "assimp.lib",
        "lua54.lib", 
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

project "MangoRuntime"
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
        "%{prj.name}/src/Shaders/**_vs.hlsl",
        "%{prj.name}/src/Shaders/**_gs.hlsl",
        "%{prj.name}/src/Shaders/**_ps.hlsl",
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.Lua}",
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
        "%{prj.name}/src/Shaders/**_vs.hlsl",
        "%{prj.name}/src/Shaders/**_gs.hlsl",
        "%{prj.name}/src/Shaders/**_ps.hlsl",
        "%{prj.name}/vendor/ImGuizmo/*.h",
        "%{prj.name}/vendor/ImGuizmo/*.cpp",
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib"
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.Lua}",
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
        "%{prj.name}/src/Shaders/**_vs.hlsl",
        "%{prj.name}/src/Shaders/**_gs.hlsl",
        "%{prj.name}/src/Shaders/**_ps.hlsl",
    }

    includedirs {
        "Mango/src",
        "%{Includes.spdlog}",
        "%{Includes.ImGui}",
        "%{Includes.Lua}",
        "Tests/vendor/catch2"
    }

    links {
        "Mango",
        "d3d11.lib",
        "dxgi.lib",
        "d3dcompiler.lib"
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