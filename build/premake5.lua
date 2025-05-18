newoption
{
	trigger = "graphics",
	value = "OPENGL_VERSION",
	description = "version of OpenGL to build raylib against",
	allowed = {
		{ "opengl11", "OpenGL 1.1"},
		{ "opengl21", "OpenGL 2.1"},
		{ "opengl33", "OpenGL 3.3"},
		{ "opengl43", "OpenGL 4.3"},
		{ "openges2", "OpenGL ES2"},
		{ "openges3", "OpenGL ES3"}
	},
	default = "opengl33"
}

function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end

function check_raylib()
    os.chdir("external")
    if(os.isdir("raylib-master") == false) then
        if(not os.isfile("raylib-master.zip")) then
            print("Raylib not found, downloading from github")
            local result_str, response_code = http.download("https://github.com/raysan5/raylib/archive/refs/heads/master.zip", "raylib-master.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("raylib-master.zip", os.getcwd())
        os.remove("raylib-master.zip")
    end
    os.chdir("../")
end

function check_assimp()
    os.chdir("external")
    if not os.isdir("assimp-master") then
        if not os.isfile("assimp-master.zip") then
            print("Assimp not found, downloading from GitHub…")
            http.download(
                "https://github.com/assimp/assimp/archive/refs/heads/master.zip",
                "assimp-master.zip",
                { progress = download_progress }
            )
        end
        print("Unzipping Assimp into " .. os.getcwd())
        zip.extract("assimp-master.zip", os.getcwd())
        os.remove("assimp-master.zip")
    end
    os.chdir("../")
end

function build_externals()
     print("calling externals")
     check_raylib()
     check_assimp()
end

function platform_defines()
    filter {"configurations:Debug or Release"}
        defines{"PLATFORM_DESKTOP"}

    filter {"configurations:Debug_RGFW or Release_RGFW"}
        defines{"PLATFORM_DESKTOP_RGFW"}

    filter {"options:graphics=opengl43"}
        defines{"GRAPHICS_API_OPENGL_43"}

    filter {"options:graphics=opengl33"}
        defines{"GRAPHICS_API_OPENGL_33"}

    filter {"options:graphics=opengl21"}
        defines{"GRAPHICS_API_OPENGL_21"}

    filter {"options:graphics=opengl11"}
        defines{"GRAPHICS_API_OPENGL_11"}

    filter {"options:graphics=openges3"}
        defines{"GRAPHICS_API_OPENGL_ES3"}

    filter {"options:graphics=openges2"}
        defines{"GRAPHICS_API_OPENGL_ES2"}

    filter {"system:macosx"}
        disablewarnings {"deprecated-declarations"}

    filter {"system:linux"}
        defines {"_GLFW_X11"}
        defines {"_GNU_SOURCE"}
    filter{}
end

-- if you don't want to download raylib, then set this to false
downloadRaylib = true
raylib_dir = "external/raylib-master"

workspaceName = 'Skinned_Raymarching_Raylib'
baseName = path.getbasename(path.getdirectory(os.getcwd()))

if (os.isdir('build_files') == false) then
    os.mkdir('build_files')
end

if (os.isdir('external') == false) then
    os.mkdir('external')
end

workspace (workspaceName)
    location "../"
    configurations { "Debug", "Release", "Debug_RGFW", "Release_RGFW"}
    platforms      { "x64", "x86", "ARM64"}
    defaultplatform ("x64")

    filter "configurations:Debug or Debug_RGFW"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release or Release_RGFW"
        defines { "NDEBUG" }
        optimize "On"

    filter { "platforms:x64" }
        architecture "x86_64"

    filter { "platforms:Arm64" }
        architecture "ARM64"

    filter {}
    targetdir "bin/%{cfg.buildcfg}/"

if (downloadRaylib) then
    build_externals()
end

startproject(workspaceName)

-- =========================================
-- ASSIMP UTILITY PROJECT
-- =========================================
project "assimp"
    kind     "Utility"
    language "C++"
    location "build_files"
    configurations { "Debug", "Release", "Debug_RGFW", "Release_RGFW" }
    platforms      { "x64", "x86", "ARM64" }

    filter "action:vs* or action:gmake*"
        prebuildcommands {
            -- 1) Ensure our per-config CMake build dir exists
            "{MKDIR} \"assimp-generated/build/%{cfg.buildcfg}\"",

            -- 2) Configure & install into bin/<cfg>/libs/assimp
            "cmake"
            .. " -S \"../external/assimp-master\""
            .. " -B \"assimp-generated/build/%{cfg.buildcfg}\""
            .. " -G \""..(_ACTION:match("vs") and "Visual Studio 17 2022" or "Unix Makefiles").."\""
            .. (_ACTION:match("vs") and " -A %{cfg.platform}" or "")
            .. " -DASSIMP_BUILD_ASSIMP_TOOLS=OFF"
            .. " -DASSIMP_BUILD_TESTS=OFF"
            .. " -DBUILD_SHARED_LIBS=OFF"
            .. " -DCMAKE_INSTALL_PREFIX=\"../../bin/%{cfg.buildcfg}/libs/assimp\"",

            -- 3) Build & INSTALL
            "cmake --build \"assimp-generated/build/%{cfg.buildcfg}\" --config %{cfg.buildcfg} --target INSTALL"
        }
    filter {}

-- =========================================
-- MAIN GAME PROJECT
-- =========================================
project (workspaceName)
    kind "ConsoleApp"
    location "build_files/"
    targetdir "../bin/%{cfg.buildcfg}"
    
    filter "action:vs*"
        debugdir "$(OutDir)"
    filter{}

    vpaths {
        ["Header Files/*"] = { "../include/**.h",  "../include/**.hpp", "../src/**.h", "../src/**.hpp"},
        ["Source Files/*"] = {"../src/**.c", "../src/**.cpp"},
    }
    files {
      "../src/**.c", "../src/**.cpp",
      "../src/**.h", "../src/**.hpp",
      "../include/**.h", "../include/**.hpp"
    }

    cdialect "C17"
    cppdialect "C++17"
    flags { "ShadowedVariables" }
    platform_defines()

    -- RAYLIB
    includedirs { "../src", "../include" }
    includedirs { raylib_dir .. "/src", raylib_dir .. "/src/external", raylib_dir .. "/src/external/glfw/include" }
    links { "raylib" }

    -- ASSIMP
    includedirs { "../bin/%{cfg.buildcfg}/libs/assimp/include" }

    filter { "system:windows" }
        libdirs { "../bin/%{cfg.buildcfg}/libs/assimp/lib" }
        libdirs {"../bin/%{cfg.buildcfg}/libs/raylib/lib"}
        links   { "winmm", "gdi32", "opengl32" }
    filter {}

    -- Link on Windows with correct per-config names
    filter { "action:vs*", "configurations:Debug or Debug_RGFW" }
        defines { "_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS" }
        dependson { "raylib", "assimp" }
        links { "raylib.lib", "assimp-vc143-mtd.lib", "zlibstaticd.lib" }
        characterset "Unicode"
        buildoptions { "/Zc:__cplusplus" }

    filter { "action:vs*", "configurations:Release or Release_RGFW" }
        defines { "_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS" }
        dependson { "raylib", "assimp" }
        links { "raylib.lib", "assimp-vc143-mt.lib", "zlibstatic.lib" }
        characterset "Unicode"
        buildoptions { "/Zc:__cplusplus" }

    -- Other platforms unchanged
    filter "system:linux"
        links { "pthread", "m", "dl", "rt", "X11" }

    filter "system:macosx"
        links { "OpenGL.framework", "Cocoa.framework", "IOKit.framework",
                "CoreFoundation.framework", "CoreAudio.framework",
                "CoreVideo.framework", "AudioToolbox.framework" }

    filter {}

    --(windows only) postbuild drop resources into the exe root folder for ease of use when wanting to disitribute later
filter { "system:windows" }
  postbuildcommands {
    "if not exist \"$(OutDir)resources\" mkdir \"$(OutDir)resources\"",
    "xcopy \"$(SolutionDir)resources\\*.*\" \"$(OutDir)resources\\\" /E /I /Y /D"
  }
filter {}



project "raylib"
    kind "StaticLib"
    language "C"
    location "build_files/"
    targetdir "../bin/%{cfg.buildcfg}/libs/raylib/lib"
    platform_defines()

    filter "action:vs*"
        defines { "_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS" }
        characterset "Unicode"
        buildoptions { "/Zc:__cplusplus" }
    filter {}

    includedirs { raylib_dir .. "/src", raylib_dir .. "/src/external/glfw/include" }
    vpaths {
        ["Header Files"] = { raylib_dir .. "/src/**.h" },
        ["Source Files/*"] = { raylib_dir .. "/src/**.c" },
    }
    files { raylib_dir .. "/src/*.h", raylib_dir .. "/src/*.c" }
    removefiles { raylib_dir .. "/src/rcore_*.c" }

    filter { "system:macosx", "files:" .. raylib_dir .. "/src/rglfw.c" }
        compileas "Objective-C"
    filter {}
