
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

	language "C"

SOLUTION_DIR = path.getabsolute("..")
dofile "toolchain.lua"
toolchain(path.join(SOLUTION_DIR, "build"), "")

--dofile "**.lua"

PROJ_DIR = SOLUTION_DIR
project "client"
	kind "ConsoleApp" --kind "StaticLib"

	debugdir (path.join(PROJ_DIR, "src"))
		
	includedirs {
		path.join(PROJ_DIR, "src"),	
		path.join(PROJ_DIR, "src/socket"),	
		path.join(PROJ_DIR, "src/utils"),		
	}

	files {
		path.join(PROJ_DIR, "src/**.c"),
		path.join(PROJ_DIR, "src/**.h"),
	}
	excludes {
		path.join(PROJ_DIR, "src/test_server.c"),
	}

	configuration { "vs* or mingw*" }
		defines {
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}
		links {
			"wsock32",
		}

	configuration { "osx" }
		--[[links {
			"Cocoa.framework",
		}]]

	configuration {}

	--strip()

project "server"
	kind "ConsoleApp" --kind "StaticLib"

	debugdir (path.join(PROJ_DIR, "src"))
		
	includedirs {
		path.join(PROJ_DIR, "src"),	
		path.join(PROJ_DIR, "src/socket"),	
		path.join(PROJ_DIR, "src/utils"),		
	}

	files {
		path.join(PROJ_DIR, "src/**.c"),
		path.join(PROJ_DIR, "src/**.h"),
	}
	excludes {
		path.join(PROJ_DIR, "src/test_client.c"),
	}

	configuration { "vs* or mingw*" }
		defines {
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}
		links {
			"wsock32",
		}

	configuration { "osx" }
		--[[links {
			"Cocoa.framework",
		}]]

	configuration {}