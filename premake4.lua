function newplatform(plf)
	local name = plf.name
	local description = plf.description

	-- Register new platform
	premake.platforms[name] = {
		cfgsuffix = "_"..name,
		iscrosscompiler = true
	}

	-- Allow use of new platform in --platfroms
	table.insert(premake.option.list["platform"].allowed, { name, description })
	table.insert(premake.fields.platforms.allowed, name)

	-- Add compiler support
	premake.gcc.platforms[name] = plf.gcc
end

function newgcctoolchain(toolchain)
	newplatform {
		name = toolchain.name,
		description = toolchain.description,
		gcc = {
			cc = toolchain.prefix .. "gcc",
			cxx = toolchain.prefix .. "g++",
			ar = toolchain.prefix .. "ar",
			cppflags = "-MMD " .. toolchain.cppflags
		}
	}
end

function newclangtoolchain(toolchain)
	newplatform {
		name = toolchain.name,
		description = toolchain.description,
		gcc = {
			cc = toolchain.prefix .. "clang",
			cxx = toolchain.prefix .. "clang++",
			ar = toolchain.prefix .. "ar",
			cppflags = "-MMD " .. toolchain.cppflags
		}
	}
end

newplatform {
	name = "clang",
	description = "Clang",
	gcc = {
		cc = "clang",
		cxx = "clang++",
		ar = "ar",
		cppflags = "-MMD "
	}
}

newplatform {
	name = "clang-static-analyze",
	description = "Clang static analysis build",
	gcc = {
		cc = "clang --analyze",
		cxx = "clang++ --analyze",
		ar = "ar",
		cppflags = "-MMD"
	}
}

newplatform {
	name = "emscripten",
	description = "Emscripten",
	gcc = {
		cc = "emcc",
		cxx = "em++",
		ar = "emar",
		cppflags = "-MMD -D__emscripten__"
	}
}

newgcctoolchain {
	name = "mingw32",
	description = "Mingw32 to cross-compile windows binaries from *nix",
	prefix = "i686-w64-mingw32-",
	cppflags = ""
}

newgcctoolchain {
	name ="android-arm7",
	description = "Android ARMv7 (not implemented)",
	prefix = iif( os.getenv("ANDROID_NDK"), os.getenv("ANDROID_NDK"), "" ) .. "arm-linux-androideabi-",
	cppflags = "-MMD -arch=armv7 -march=armv7 -marm -mcpu=cortex-a8"
}

toolchain_path = os.getenv("TOOLCHAINPATH")

if not toolchain_path then
	toolchain_path = ""
end

-- cross compiling from linux, totally experimental, using: http://code.google.com/p/ios-toolchain-based-on-clang-for-linux/
newplatform {
	name = "ios-cross-arm7",
	description = "iOS ARMv7 ( cross-compiling )",
	gcc = {
		cc = "ios-clang",
		cxx = "ios-clang++",
		ar = "arm-apple-darwin11-ar",
		cppflags = "-MMD -march=armv7 -marm -mcpu=cortex-a8"
	}
}

newplatform {
	name = "ios-cross-x86",
	description = "iOS x86 ( cross-compiling )",
	gcc = {
		cc = "ios-clang",
		cxx = "ios-clang++",
		ar = "arm-apple-darwin11-ar",
		cppflags = "-MMD -march=i386 -m32"
	}
}

newclangtoolchain {
	name ="ios-arm7",
	description = "iOS ARMv7",
	prefix = iif( os.getenv("TOOLCHAINPATH"), os.getenv("TOOLCHAINPATH"), "" ),
	cppflags = "-arch armv7 -mfpu=neon"
}

newclangtoolchain {
	name ="ios-x86",
	description = "iOS x86",
	prefix = iif( os.getenv("TOOLCHAINPATH"), os.getenv("TOOLCHAINPATH"), "" ),
	cppflags = "-m32 -arch i386"
}

if _OPTIONS.platform then
	-- overwrite the native platform with the options::platform
	premake.gcc.platforms['Native'] = premake.gcc.platforms[_OPTIONS.platform]
end

newoption { trigger = "with-openssl", description = "Enables OpenSSL support ( and disables mbedtls backend )." }
newoption { trigger = "with-dynamic-freetype", description = "Dynamic link against freetype." }
newoption { trigger = "with-static-eepp", description = "Force to build the demos and tests with eepp compiled statically" }
newoption { trigger = "with-static-backend", description = "It will try to compile the library with a static backend (only for gcc and mingw).\n\t\t\t\tThe backend should be placed in libs/your_platform/libYourBackend.a" }
newoption { trigger = "with-gles2", description = "Compile with GLES2 support" }
newoption { trigger = "with-gles1", description = "Compile with GLES1 support" }
newoption { trigger = "use-frameworks", description = "In macOS it will try to link the external libraries from its frameworks. For example, instead of linking against SDL2 it will link agains SDL2.framework." }
newoption {
	trigger = "with-backend",
	description = "Select the backend to use for window and input handling.\n\t\t\tIf no backend is selected or if the selected is not installed the script will search for a backend present in the system, and will use it.",
	allowed = {
		{ "SDL2",  "SDL2" }
	}
}

function explode(div,str)
    if (div=='') then return false end
    local pos,arr = 0,{}
    for st,sp in function() return string.find(str,div,pos,true) end do
        table.insert(arr,string.sub(str,pos,st-1))
        pos = sp + 1
    end
    table.insert(arr,string.sub(str,pos))
    return arr
end

function os.get_real()
	if 	_OPTIONS.platform == "ios-arm7" or
		_OPTIONS.platform == "ios-x86" or
		_OPTIONS.platform == "ios-cross-arm7" or
		_OPTIONS.platform == "ios-cross-x86" then
		return "ios"
	end

	if _OPTIONS.platform == "android-arm7" then
		return "android"
	end

	if 	_OPTIONS.platform == "mingw32" then
		return _OPTIONS.platform
	end

	if 	_OPTIONS.platform == "emscripten" then
		return _OPTIONS.platform
	end

	return os.get()
end

function os.is_real( os_name )
	return os.get_real() == os_name
end

if os.is_real("haiku") and not os.is64bit() then
	premake.gcc.cc = "gcc-x86"
	premake.gcc.cxx = "g++-x86"
	premake.gcc.ar = "ar-x86"
end

function print_table( table_ref )
	for _, value in pairs( table_ref ) do
		print(value)
	end
end

function table_length(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

function args_contains( element )
	return table.contains( _ARGS, element )
end

function multiple_insert( parent_table, insert_table )
	for _, value in pairs( insert_table ) do
		table.insert( parent_table, value )
	end
end

function get_ios_arch()
	local archs = explode( "-", _OPTIONS.platform )
	return archs[ table_length( archs ) ]
end

function os_findlib( name )
	if os.is_real("macosx") and ( is_xcode() or _OPTIONS["use-frameworks"] ) then
		local path = "/Library/Frameworks/" .. name .. ".framework"

		if os.isdir( path ) then
			return path
		end
	end

	return os.findlib( name )
end

function get_backend_link_name( name )
	if os.is_real("macosx") and ( is_xcode() or _OPTIONS["use-frameworks"] ) then
		local fname = name .. ".framework"

		if os_findlib( name ) then -- Search for the framework
			return fname
		end
	end

	return name
end

function string.starts(String,Start)
	if ( _ACTION ) then
		return string.sub(String,1,string.len(Start))==Start
	end

	return false
end

function is_vs()
	return ( string.starts(_ACTION,"vs") )
end

function is_xcode()
	return ( string.starts(_ACTION,"xcode") )
end

function set_kind()
	if os.is_real("macosx") then
		kind("ConsoleApp")
	else
		kind("WindowedApp")
	end
end

link_list = { }
os_links = { }
backends = { }
static_backends = { }
backend_selected = false

function build_base_configuration( package_name )
	includedirs { "src/thirdparty/zlib" }

	if not os.is("windows") then
		buildoptions{ "-fPIC" }
	end

	if is_vs() then
		includedirs { "src/thirdparty/libzip/vs" }
	end

	configuration "debug"
		defines { "DEBUG" }
		flags { "Symbols" }
		if not is_vs() then
			buildoptions{ "-Wall", "-std=gnu99" }
		end
		targetname ( package_name .. "-debug" )

	configuration "release"
		defines { "NDEBUG" }
		flags { "OptimizeSpeed" }
		if not is_vs() then
			buildoptions{ "-Wall", "-std=gnu99" }
		end
		targetname ( package_name )

	set_ios_config()
	set_xcode_config()
end

function build_base_cpp_configuration( package_name )
	if not os.is("windows") then
		buildoptions{ "-fPIC" }
	end

	set_ios_config()
	set_xcode_config()

	configuration "debug"
		defines { "DEBUG" }
		flags { "Symbols" }
		if not is_vs() then
			buildoptions{ "-Wall" }
		end
		targetname ( package_name .. "-debug" )

	configuration "release"
		defines { "NDEBUG" }
		flags { "OptimizeSpeed" }
		if not is_vs() then
			buildoptions{ "-Wall" }
		end
		targetname ( package_name )
end

function add_cross_config_links()
	if not is_vs() then
		if os.is_real("mingw32") or os.is_real("windows") or os.is_real("ios") then -- if is crosscompiling from *nix
			linkoptions { "-static-libgcc", "-static-libstdc++" }
		end

		if os.is_real("mingw32") then
			linkoptions { "-Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic" }
		end
	end
end

function fix_shared_lib_linking_path( package_name, libname )
	if ( "4.4-beta5" == _PREMAKE_VERSION or "HEAD" == _PREMAKE_VERSION ) and not _OPTIONS["with-static-eepp"] and package_name == "eepp" then
		if os.is("macosx") then
			linkoptions { "-install_name " .. libname .. ".dylib" }
		elseif os.is("linux") or os.is("freebsd") then
			linkoptions { "-Wl,-soname=\"" .. libname .. "\"" }
		elseif os.is("haiku") then
			linkoptions { "-Wl,-soname=\"" .. libname .. ".so" .. "\"" }
		end
	end
end

function build_link_configuration( package_name, use_ee_icon )
	includedirs { "include" }

	local extension = "";

	if package_name == "eepp" then
		defines { "EE_EXPORTS" }
	elseif package_name == "eepp-static" then
		defines { "EE_STATIC" }
	end

	if not is_vs() then
		buildoptions{ "-std=c++14" }
	end

	if package_name ~= "eepp" and package_name ~= "eepp-static" then
		if not _OPTIONS["with-static-eepp"] then
			links { "eepp-shared" }
		else
			links { "eepp-static" }
			defines { "EE_STATIC" }
			add_static_links()
			links { link_list }
		end

		if os.is("windows") and not is_vs() then
			if ( true == use_ee_icon ) then
				linkoptions { "../../bin/assets/icon/ee.res" }
			end
		end

		if os.is_real("emscripten") then
			extension = ".html"

			if (	package_name ~= "eeew" and
					package_name ~= "eees" and
					package_name ~= "eehttp-request" and
					package_name ~= "eephysics" and
					package_name ~= "eevbo-fbo-batch"
			) then
				linkoptions { "--preload-file assets/" }
			end
		end

		if _OPTIONS.platform == "ios-cross-arm7" then
			extension = ".ios"
		end

		if _OPTIONS.platform == "ios-cross-x86" then
			extension = ".x86.ios"
		end
	end

	configuration "debug"
		defines { "DEBUG", "EE_DEBUG", "EE_MEMORY_MANAGER" }
		flags { "Symbols" }

		if not is_vs() and not os.is_real("emscripten") then
			buildoptions{ "-Wall -Wno-long-long" }
		end

		fix_shared_lib_linking_path( package_name, "libeepp-debug" )

		targetname ( package_name .. "-debug" .. extension )

	configuration "release"
		defines { "NDEBUG" }
		flags { "OptimizeSpeed" }

		if not is_vs() and not os.is_real("emscripten") then
			buildoptions { "-fno-strict-aliasing -ffast-math" }
		end

		if not is_vs() and not os.is_real("emscripten") and not os.is_real("macosx") then
			buildoptions { "-s" }
		end

		fix_shared_lib_linking_path( package_name, "libeepp" )

		targetname ( package_name .. extension )

	configuration "windows"
		add_cross_config_links()

	configuration "emscripten"
		linkoptions{ "-O2 -s TOTAL_MEMORY=67108864 -s ASM_JS=1 -s VERBOSE=1 -s DISABLE_EXCEPTION_CATCHING=0 -s USE_SDL=2 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s FULL_ES3=1 -s \"BINARYEN_TRAP_MODE='clamp'\"" }
		buildoptions { "-fno-strict-aliasing -O2 -s USE_SDL=2 -s PRECISE_F32=1" }

		if _OPTIONS["with-gles1"] and ( not _OPTIONS["with-gles2"] or _OPTIONS["force-gles1"] ) then
			linkoptions{ "-s LEGACY_GL_EMULATION=1" }
		end

		if _OPTIONS["with-gles2"] and not _OPTIONS["force-gles1"] then
			linkoptions{ "-s FULL_ES2=1" }
		end

	set_ios_config()
	set_xcode_config()
end

function generate_os_links()
	if os.is_real("linux") then
		multiple_insert( os_links, { "rt", "pthread", "X11", "openal", "GL", "Xcursor" } )

		if _OPTIONS["with-static-eepp"] then
			table.insert( os_links, "dl" )
		end
	elseif os.is_real("windows") then
		multiple_insert( os_links, { "OpenAL32", "opengl32", "glu32", "gdi32", "ws2_32", "winmm" } )
	elseif os.is_real("mingw32") then
		multiple_insert( os_links, { "OpenAL32", "opengl32", "glu32", "gdi32", "ws2_32", "winmm" } )
	elseif os.is_real("macosx") then
		multiple_insert( os_links, { "OpenGL.framework", "OpenAL.framework", "CoreFoundation.framework" } )
	elseif os.is_real("freebsd") then
		multiple_insert( os_links, { "rt", "pthread", "X11", "openal", "GL", "Xcursor" } )
	elseif os.is_real("haiku") then
		multiple_insert( os_links, { "openal", "GL", "network" } )
	elseif os.is_real("ios") then
		multiple_insert( os_links, { "OpenGLES.framework", "OpenAL.framework", "AudioToolbox.framework", "CoreAudio.framework", "Foundation.framework", "CoreFoundation.framework", "UIKit.framework", "QuartzCore.framework", "CoreGraphics.framework" } )
	elseif os.is_real("emscripten") then
		multiple_insert( os_links, { "openal" } )
	end
end

function parse_args()
	if _OPTIONS["with-gles2"] then
		defines { "EE_GLES2", "SOIL_GLES2" }
	end

	if _OPTIONS["with-gles1"] then
		defines { "EE_GLES1", "SOIL_GLES1" }
	end
end

function add_static_links()
	-- The linking order DOES matter
	-- Expose the symbols that need one static library AFTER adding that static lib

	-- Add static backends
	if next(static_backends) ~= nil then
		for _, value in pairs( static_backends ) do
			linkoptions { value }
		end
	end

	if not _OPTIONS["with-dynamic-freetype"] then
		links { "freetype-static" }
	end

	links { "SOIL2-static",
			"chipmunk-static",
			"libzip-static",
			"jpeg-compressor-static",
			"zlib-static",
			"imageresampler-static",
			"pugixml-static",
			"vorbis-static"
	}

	if not _OPTIONS["with-openssl"] then
		links { "mbedtls-static" }
	end

	if not os.is_real("haiku") and not os.is_real("ios") and not os.is_real("android") and not os.is_real("emscripten") then
		links{ "glew-static" }
	end
end

function can_add_static_backend( name )
	if _OPTIONS["with-static-backend"] then
		local path = "libs/" .. os.get_real() .. "/lib" .. name .. ".a"
		return os.isfile(path)
	end
end

function insert_static_backend( name )
	table.insert( static_backends, path.getrelative( "libs/" .. os.get_real(), "./" ) .. "/libs/" .. os.get_real() .. "/lib" .. name .. ".a" )
end

function add_sdl2()
	print("Using SDL2 backend");
	files { "src/eepp/window/backend/SDL2/*.cpp" }
	defines { "EE_BACKEND_SDL_ACTIVE", "EE_SDL_VERSION_2" }

	if not can_add_static_backend("SDL2") then
		table.insert( link_list, get_backend_link_name( "SDL2" ) )
	else
		insert_static_backend( "SDL2" )
	end
end

function set_xcode_config()
	if is_xcode() or _OPTIONS["use-frameworks"] then
		linkoptions { "-F/Library/Frameworks" }
		includedirs { "/Library/Frameworks/SDL2.framework/Headers" }
		defines { "EE_SDL2_FROM_ROOTPATH" }
	end
end

function set_ios_config()
	if _OPTIONS.platform == "ios-arm7" or _OPTIONS.platform == "ios-x86" then
		local err = false

		if nil == os.getenv("TOOLCHAINPATH") then
			print("You must set TOOLCHAINPATH enviroment variable.")
			print("\tExample: /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/usr/bin/")
			err = true
		end

		if nil == os.getenv("SYSROOTPATH") then
			print("You must set SYSROOTPATH enviroment variable.")
			print("\tExample: /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS5.0.sdk")
			err = true
		end

		if nil == os.getenv("IOSVERSION") then
			print("You must set IOSVERSION enviroment variable.")
			print("\tExample: 5.0")
			err = true
		end

		if err then
			os.exit(1)
		end

		local sysroot_path = os.getenv("SYSROOTPATH")
		local framework_path = sysroot_path .. "/System/Library/Frameworks"
		local framework_libs_path = framework_path .. "/usr/lib"
		local sysroot_ver = " -miphoneos-version-min=" .. os.getenv("IOSVERSION") .. " -isysroot " .. sysroot_path

		buildoptions { sysroot_ver .. " -I" .. sysroot_path .. "/usr/include" }
		linkoptions { sysroot_ver }
		libdirs { framework_libs_path }
		linkoptions { " -F" .. framework_path .. " -L" .. framework_libs_path .. " -isysroot " .. sysroot_path }
		includedirs { "src/thirdparty/SDL2/include" }
	end

	if _OPTIONS.platform == "ios-cross-arm7" or _OPTIONS.platform == "ios-cross-x86" then
		includedirs { "src/thirdparty/SDL2/include" }
	end
end

function backend_is( name, libname )
	if not _OPTIONS["with-backend"] then
		_OPTIONS["with-backend"] = "SDL2"
	end

	if next(backends) == nil then
		backends = string.explode(_OPTIONS["with-backend"],",")
	end

	local backend_sel = table.contains( backends, name )

	local ret_val = os_findlib( libname ) and backend_sel

	if os.is_real("mingw32") or os.is_real("emscripten") then
		ret_val = backend_sel
	end

	if ret_val then
		backend_selected = true
	end

	return ret_val
end

function select_backend()
	if backend_is("SDL2", "SDL2") then
		print("Selected SDL2")
		add_sdl2()
	end

	-- If the selected backend is not present, try to find one present
	if not backend_selected then
		if os_findlib("SDL2", "SDL2") then
			add_sdl2()
		else
			print("ERROR: Couldnt find any backend. Forced SDL2.")
			add_sdl2( true )
		end
	end
end

function check_ssl_support()
	if _OPTIONS["with-openssl"] then
		if os.is("windows") then
			table.insert( link_list, get_backend_link_name( "libssl" ) )
			table.insert( link_list, get_backend_link_name( "libcrypto" ) )
		else
			table.insert( link_list, get_backend_link_name( "ssl" ) )
			table.insert( link_list, get_backend_link_name( "crypto" ) )
		end

		files { "src/eepp/network/ssl/backend/openssl/*.cpp" }

		defines { "EE_OPENSSL" }
	else
		files { "src/eepp/network/ssl/backend/mbedtls/*.cpp" }

		defines { "EE_MBEDTLS" }
	end

	defines { "EE_SSL_SUPPORT" }
end

function build_eepp( build_name )
	includedirs { "include", "src", "src/thirdparty", "include/eepp/thirdparty", "src/thirdparty/freetype2/include", "src/thirdparty/zlib", "src/thirdparty/libogg/include", "src/thirdparty/libvorbis/include", "src/thirdparty/mbedtls/include" }

	set_ios_config()
	set_xcode_config()

	add_static_links()

	if is_vs() then
		includedirs { "src/thirdparty/libzip/vs" }
	end

	if not is_vs() then
		buildoptions{ "-std=c++11" }
	end

	if os.is("windows") then
		files { "src/eepp/system/platform/win/*.cpp" }
		files { "src/eepp/network/platform/win/*.cpp" }
	else
		files { "src/eepp/system/platform/posix/*.cpp" }
		files { "src/eepp/network/platform/unix/*.cpp" }
	end

	files { "src/eepp/core/*.cpp",
			"src/eepp/math/*.cpp",
			"src/eepp/system/*.cpp",
			"src/eepp/audio/*.cpp",
			"src/eepp/graphics/*.cpp",
			"src/eepp/graphics/renderer/*.cpp",
			"src/eepp/window/*.cpp",
			"src/eepp/network/*.cpp",
			"src/eepp/network/ssl/*.cpp",
			"src/eepp/network/http/*.cpp",
			"src/eepp/scene/*.cpp",
			"src/eepp/scene/actions/*.cpp",
			"src/eepp/ui/*.cpp",
			"src/eepp/ui/actions/*.cpp",
			"src/eepp/ui/css/*.cpp",
			"src/eepp/ui/tools/*.cpp",
			"src/eepp/physics/*.cpp",
			"src/eepp/physics/constraints/*.cpp",
			"src/eepp/maps/*.cpp",
			"src/eepp/maps/mapeditor/*.cpp"
	}

	check_ssl_support()

	select_backend()

	if _OPTIONS["with-dynamic-freetype"] and os_findlib("freetype") then
		table.insert( link_list, get_backend_link_name( "freetype" ) )
	end

	multiple_insert( link_list, os_links )

	links { link_list }

	build_link_configuration( build_name )

	configuration "emscripten"
		if _OPTIONS["force-gles1"] then
			defines{ "EE_GLES1_DEFAULT" }
		end
end

function set_targetdir( dir )
	if os.is_real("ios") then
		targetdir(dir .. get_ios_arch() .. "/" )
	else
		targetdir(dir)
	end
end

solution "eepp"

	targetdir("./bin/")
	configurations { "debug", "release" }

	if os.is_real("ios") then
		location("./make/" .. _OPTIONS.platform .. "/" )
		objdir("obj/" .. os.get_real() .. "/" .. get_ios_arch() .. "/" )
	else
		location("./make/" .. os.get_real() .. "/")
		objdir("obj/" .. os.get_real() .. "/")
	end

	generate_os_links()
	parse_args()

	project "SOIL2-static"
		kind "StaticLib"

		if is_vs() then
			language "C++"
			buildoptions { "/TP" }
		else
			language "C"
		end

		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/SOIL2/src/SOIL2/*.c" }
		includedirs { "src/thirdparty/SOIL2" }
		build_base_configuration( "SOIL2" )

	if not os.is_real("haiku") and not os.is_real("ios") and not os.is_real("android") and not os.is_real("emscripten") then
		project "glew-static"
			kind "StaticLib"
			language "C"
			defines { "GLEW_NO_GLU", "GLEW_STATIC" }
			set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
			files { "src/thirdparty/glew/*.c" }
			includedirs { "include/thirdparty/glew" }
			build_base_configuration( "glew" )
	end

	if not _OPTIONS["with-openssl"] then
		project "mbedtls-static"
			kind "StaticLib"
			language "C"
			set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
			includedirs { "src/thirdparty/mbedtls/include/" }
			files { "src/thirdparty/mbedtls/library/*.c" }
			build_base_cpp_configuration( "mbedtls" )
	end

	project "vorbis-static"
		kind "StaticLib"
		language "C"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		includedirs { "src/thirdparty/libvorbis/lib/", "src/thirdparty/libogg/include", "src/thirdparty/libvorbis/include" }
		files { "src/thirdparty/libogg/**.c", "src/thirdparty/libvorbis/**.c" }
		build_base_cpp_configuration( "vorbis" )

	project "pugixml-static"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/pugixml/*.cpp" }
		build_base_cpp_configuration( "pugixml" )

	project "zlib-static"
		kind "StaticLib"
		language "C"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/zlib/*.c" }
		build_base_configuration( "zlib" )

	project "libzip-static"
		kind "StaticLib"
		language "C"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/libzip/*.c" }
		includedirs { "src/thirdparty/zlib" }
		build_base_configuration( "libzip" )

	project "freetype-static"
		kind "StaticLib"
		language "C"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		defines { "FT2_BUILD_LIBRARY" }
		files { "src/thirdparty/freetype2/src/**.c" }
		includedirs { "src/thirdparty/freetype2/include" }
		build_base_configuration( "freetype" )

	project "chipmunk-static"
		kind "StaticLib"

		if is_vs() then
			language "C++"
			buildoptions { "/TP" }
		else
			language "C"
		end

		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/chipmunk/*.c", "src/thirdparty/chipmunk/constraints/*.c" }
		includedirs { "include/eepp/thirdparty/chipmunk" }
		build_base_configuration( "chipmunk" )

	project "jpeg-compressor-static"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/jpeg-compressor/*.cpp" }
		build_base_cpp_configuration( "jpeg-compressor" )

	project "imageresampler-static"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		files { "src/thirdparty/imageresampler/*.cpp" }
		build_base_cpp_configuration( "imageresampler" )

	project "efsw-static"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/thirdparty/")
		includedirs { "src/thirdparty/efsw/include", "src/thirdparty/efsw/src" }

		if os.is("windows") then
			osfiles = "src/thirdparty/efsw/src/efsw/platform/win/*.cpp"
		else
			osfiles = "src/thirdparty/efsw/src/efsw/platform/posix/*.cpp"
		end

		files { "src/thirdparty/efsw/src/efsw/*.cpp", osfiles }

		if os.is("windows") then
			excludes { "src/thirdparty/efsw/src/efsw/WatcherKqueue.cpp", "src/thirdparty/efsw/src/efsw/WatcherFSEvents.cpp", "src/thirdparty/efsw/src/efsw/WatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherKqueue.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherFSEvents.cpp" }
		elseif os.is("linux") then
			excludes { "src/thirdparty/efsw/src/efsw/WatcherKqueue.cpp", "src/thirdparty/efsw/src/efsw/WatcherFSEvents.cpp", "src/thirdparty/efsw/src/efsw/WatcherWin32.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherKqueue.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherWin32.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherFSEvents.cpp" }
		elseif os.is("macosx") then
			excludes { "src/thirdparty/efsw/src/efsw/WatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/WatcherWin32.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherWin32.cpp" }
		elseif os.is("freebsd") then
			excludes { "src/thirdparty/efsw/src/efsw/WatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/WatcherWin32.cpp", "src/thirdparty/efsw/src/efsw/WatcherFSEvents.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherInotify.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherWin32.cpp", "src/thirdparty/efsw/src/efsw/FileWatcherFSEvents.cpp" }
		end

		build_base_cpp_configuration( "efsw" )

	project "eepp-main"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/")
		files { "src/eepp/main/eepp_main.cpp" }

	project "eepp-static"
		kind "StaticLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/")
		build_eepp( "eepp-static" )

	project "eepp-shared"
		kind "SharedLib"
		language "C++"
		set_targetdir("libs/" .. os.get_real() .. "/")
		build_eepp( "eepp" )

	-- Examples
	project "eepp-test"
		set_kind()
		language "C++"
		files { "src/test/*.cpp" }
		build_link_configuration( "eetest", true )

	project "eepp-es"
		set_kind()
		language "C++"
		files { "src/examples/external_shader/*.cpp" }
		build_link_configuration( "eees", true )

	project "eepp-ew"
		set_kind()
		language "C++"
		files { "src/examples/empty_window/*.cpp" }
		build_link_configuration( "eeew", true )

	project "eepp-sound"
		kind "ConsoleApp"
		language "C++"
		files { "src/examples/sound/*.cpp" }
		build_link_configuration( "eesound", true )

	project "eepp-sprites"
		set_kind()
		language "C++"
		files { "src/examples/sprites/*.cpp" }
		build_link_configuration( "eesprites", true )

	project "eepp-fonts"
		set_kind()
		language "C++"
		files { "src/examples/fonts/*.cpp" }
		build_link_configuration( "eefonts", true )

	project "eepp-vbo-fbo-batch"
		set_kind()
		language "C++"
		files { "src/examples/vbo_fbo_batch/*.cpp" }
		build_link_configuration( "eevbo-fbo-batch", true )

	project "eepp-physics"
		set_kind()
		language "C++"
		files { "src/examples/physics/*.cpp" }
		build_link_configuration( "eephysics", true )

	project "eepp-http-request"
		kind "ConsoleApp"
		language "C++"
		files { "src/examples/http_request/*.cpp" }
		includedirs { "src/thirdparty" }
		build_link_configuration( "eehttp-request", true )

	-- Tools
	project "eepp-textureatlaseditor"
		set_kind()
		language "C++"
		files { "src/tools/textureatlaseditor/*.cpp" }
		build_link_configuration( "eepp-TextureAtlasEditor", true )

	project "eepp-mapeditor"
		set_kind()
		language "C++"
		files { "src/tools/mapeditor/*.cpp" }
		build_link_configuration( "eepp-MapEditor", true )

	project "eepp-uieditor"
		set_kind()
		language "C++"
		includedirs { "src/thirdparty/efsw/include", "src/thirdparty" }

		if not os.is("windows") and not os.is("haiku") then
			links { "pthread" }
		end

		links { "efsw-static", "pugixml-static" }
		files { "src/tools/uieditor/*.cpp" }
		build_link_configuration( "eepp-UIEditor", true )

if os.isfile("external_projects.lua") then
	dofile("external_projects.lua")
end
