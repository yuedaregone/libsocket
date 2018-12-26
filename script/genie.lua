
solution "testcp"
	configurations {
		"Debug",
		"Release",
	}

	platforms {
--		"x32",
--		"x64",
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
		path.join(PROJ_DIR, "src/net/socket"),
		path.join(PROJ_DIR, "src/net/msgs"),
		path.join(PROJ_DIR, "src/config"),	
		path.join(PROJ_DIR, "src/utils"),		
	}

	files {
		path.join(PROJ_DIR, "src/**.c"),
		path.join(PROJ_DIR, "src/**.h"),
	}
	excludes {
		path.join(PROJ_DIR, "src/test_server.c"),
		path.join(PROJ_DIR, "src/test_client.c"),
		path.join(PROJ_DIR, "src/net/net_server.c"),
		path.join(PROJ_DIR, "src/net/http_server.c"),
	}

	configuration { "vs* or mingw*" }
		defines {
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}
		links {
			"wsock32",
		}

	configuration { "osx" }
		links {
			--"Cocoa.framework",
			--"CFNetwork.framework",
		}
	configuration { "linux" }
		buildoptions {
			"-std=c99",
			"-Wlogical-op",
		}	

	configuration {}

	--strip()

project "server"
	kind "ConsoleApp" --kind "StaticLib"

	debugdir (path.join(PROJ_DIR, "src"))
		
	includedirs {
		path.join(PROJ_DIR, "src"),	
		path.join(PROJ_DIR, "src/net/socket"),
		path.join(PROJ_DIR, "src/net/msgs"),
		path.join(PROJ_DIR, "src/config"),	
		path.join(PROJ_DIR, "src/utils"),		
	}

	files {
		path.join(PROJ_DIR, "src/**.c"),
		path.join(PROJ_DIR, "src/**.h"),
	}
	excludes {
		path.join(PROJ_DIR, "src/test_server.c"),
		path.join(PROJ_DIR, "src/test_client.c"),
		path.join(PROJ_DIR, "src/net/net_client.c"),
		path.join(PROJ_DIR, "src/net/http_server.c"),
	}

	configuration { "vs* or mingw*" }
		defines {
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		}
		links {
			"wsock32",
		}

	configuration { "osx" }
		links {
			--"Cocoa.framework",
			--"CFNetwork.framework",
		}

	configuration { "linux" }
		buildoptions {
			"-std=c99",
			"-Wlogical-op",
		}
	configuration {}
