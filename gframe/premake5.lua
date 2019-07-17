local stuff=function(static_core)
	kind "ConsoleApp"
	files { "**.cpp", "**.cc", "**.c", "**.h" }
	excludes "lzma/**"
	includedirs { "../ocgcore" }

	if static_core then
		links "ocgcore"
	end

	links { "clzma", "sqlite3" , "event" }
	filter "system:windows"
		includedirs { "../event/include", "../sqlite3" }
		links { "opengl32", "ws2_32", "winmm", "gdi32", "kernel32", "user32", "imm32" }

	filter "action:not vs*"
		buildoptions { "-std=c++14", "-fno-rtti", "-fpermissive" }

	filter "system:not windows"
		defines "LUA_COMPAT_5_2"
		excludes "COSOperator.*"
		links { "event_pthreads", "GL", "dl", "pthread" }
		if static_core then
			links  "lua5.3-c++"
		end
		linkoptions { "-Wl,-rpath '-Wl,$$ORIGIN'" }

	if static_core then
		filter "system:bsd"
			defines "LUA_USE_POSIX"

		filter "system:macosx"
			defines "LUA_USE_MACOSX"

		filter "system:linux"
			defines "LUA_USE_LINUX"
	end
end

include "lzma/."
project "ygopro"
	targetname "ygopro"
	stuff(true)
		
project "ygoprodll"
	targetname "ygoprodll"
	defines "YGOPRO_BUILD_DLL"
	stuff()