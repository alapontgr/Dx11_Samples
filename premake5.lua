config = {}
config.Platforms = { "Win64" }

config.frameworkSrc = os.realpath("./framework")
config.samplesSrcBase = os.realpath("./samples")
config.externalSrc = os.realpath("./external")

config.OutDir = os.realpath("./_bin")
config.ObjDir = os.realpath("./_obj")
config.WorkingDir = os.realpath("./")
config.assetsDir = os.realpath("./assets")
config.ProjectFilesDir = os.realpath("./_vs")
config.frameworkLibDir = config.OutDir .. "/framework"

config.WorkingDirPath = config.WorkingDir .. "/" .. "%{prj.platform}" .. "/"
config.OutPath = config.OutDir .. "/" .. "%{cfg.platform}" .. "/" .. "%{cfg.buildcfg}"
config.ObjPath = config.ObjDir .. "/" .. "%{cfg.platform}" .. "/" .. "%{cfg.buildcfg}"

----------------------------------------------------------
-- Setup the solution and configure the shared settings --
----------------------------------------------------------

workspace ("Dx11_Samples")

   	configurations { "Debug", "DebugOpt", "Release"} 
  	platforms { config.Platforms }

	targetdir(config.OutPath)
	libdirs { config.OutPath }
   	location( config.ProjectFilesDir)
   	--debugdir( config.assetsDir )

	---------------------------------------------------------------   
	-- TODO: Refactor this to depend on the platform that is chosen
   	architecture "x64"
	-- entrypoint  ("WinMainCRTStartup")
	defines { "_PLATFORM_DIR=Win64" }
	system "Windows"
   	---------------------------------------------------------------

   	flags { "FatalWarnings" }

	filter { "configurations:Debug" }
		defines { "_DEBUG" }
		symbols "On"
		optimize "Off"
	
    filter { "configurations:DebugOpt" }
		defines { "_DEBUGOPT" }
		symbols "On"
		optimize "On"

	filter { "configurations:Release" }
		defines { "_RELEASE" }
		symbols "Off"
		optimize "Full"

	filter {}

----------------------------------------------------------
-- Build the solution
----------------------------------------------------------

project "framework"

ExternalProjects = 
{
	"glm",
	"stb",
	"tinygltf",
	"imgui",
	"ImGuizmo"
}

kind "StaticLib"
language "C++"

location( config.ProjectFilesDir)
targetdir(config.OutPath)
objdir(config.ObjPath .. "/framework")
-------------------
-- Add include dirs
includedirs { "./", "./external" }

-------------------
-- Add files

fileDirs = {"./framework/**.*"}

-- External files
for k, Name in pairs(ExternalProjects) do
	table.insert(fileDirs, "./external/" .. Name .. "/**.*")
end

files {fileDirs}

----------------------------------------------------------
-- Samples
----------------------------------------------------------
function addSample(sampleName)
	project(sampleName)
	kind "ConsoleApp"
	language "C++"

	links {
		"framework",
	}

	location( config.ProjectFilesDir)
	targetdir(config.OutPath .. "/" .. sampleName)
	objdir(config.ObjPath .. "/" .. sampleName .. "/obj/")
	debugdir( "./assets" )

	-------------------
	-- Add include dirs
	includedirs { "./samples", "./"}

	-------------------
	-- Add lib dirs
	libdirs { config.frameworkLibDir }

	-------------------
	-- Add files
	files {"./samples/" .. sampleName .. "/**.*"}
 end

----------------------------------------------------------
group "samples"
addSample("0_HelloTriangle")
addSample("1_Draw3DCube");
addSample("2_Texturing");
addSample("3_Hierarchy");
addSample("4_LoadingGLTF");
addSample("5_Lighting");
----------------------------------------------------------