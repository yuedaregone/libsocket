
solution "testcp"
	configurations {
		"Debug",
		"Release",
	}

	platforms {
		"x32",
		"x64",
		"Native", -- for targets where bitness is not specified
	}

	language "C++"

SOLUTION_DIR = path.getabsolute("..")
dofile "toolchain.lua"
toolchain(path.join(SOLUTION_DIR, "build"), "")

--dofile "**.lua"

PROJ_DIR = SOLUTION_DIR
project "testcp"
	kind "ConsoleApp" --kind "StaticLib"

	debugdir (path.join(PROJ_DIR, "src"))	

	
	includedirs {
		path.join(PROJ_DIR, "src"),		
	}

	files {
		path.join(PROJ_DIR, "src/testcp.cpp"),
		--path.join(PROJ_DIR, "src/**.h"),
	}
--[[
	links {
		"bx",
	}]]

	configuration { "vs* or mingw*" }
		links {
			"psapi",
		}
		buildoptions {
			"/wd4244",
		}		

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "osx" }
		links {
			"Cocoa.framework",
		}

	configuration {}

	--strip()

