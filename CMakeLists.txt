# CMake 3.22 is installed on Ubuntu 22.04 LTS, so don't want to go higher than that.
# CMake 3.24 CMAKE_COMPILE_WARNING_AS_ERROR, but is ignored on lower versions.
# CMake 3.26 made USE_FOLDERS global property default to true
cmake_minimum_required(VERSION 3.22)

# "When using vcpkg as a submodule of your project, you can add the following to your CMakeLists.txt 
# before the first project() call, instead of passing CMAKE_TOOLCHAIN_FILE to the cmake invocation.
# This will still allow people to not use vcpkg, by passing the CMAKE_TOOLCHAIN_FILE directly, but 
# it will make the configure-build step slightly easier."
# https://github.com/microsoft/vcpkg/blob/master/README.md#vcpkg-as-a-submodule-with-cmake
#
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# Version number stored in a separate file for visibility
# See Professional CMake 17th Edition Chapter 22
include(cmake/AppVersion.cmake)

# project() should be called ASAP following cmake_minimum_required()
# Sets PROJECT_NAME. Defaults to LANGUAGES C CXX. This will be the Visual Studio Solution name.
project(hoffgui VERSION ${APP_VERSION})

message(VERBOSE, "hoffgui_VERSION = ${hoffgui_VERSION}")
message(VERBOSE, "hoffgui_VERSION_MAJOR = ${hoffgui_VERSION_MAJOR}")
message(VERBOSE, "hoffgui_VERSION_MINOR = ${hoffgui_VERSION_MINOR}")
message(VERBOSE, "hoffgui_VERSION_PATCH = ${hoffgui_VERSION_PATCH}")
message(VERBOSE, "hoffgui_VERSION_TWEAK = ${hoffgui_VERSION_TWEAK}")

configure_file(cmake/AppVersion.cpp.in ${CMAKE_SOURCE_DIR}/src/AppVersion.cpp @ONLY)
configure_file(cmake/VERSION.txt.in ${CMAKE_SOURCE_DIR}/VERSION.txt @ONLY)

message(VERBOSE, "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
message(VERBOSE, "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(VERBOSE, "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(VERBOSE, "CMAKE_HOST_SYSTEM = ${CMAKE_HOST_SYSTEM}") # "Linux-6.6.31+rpt-rpi-v8" on Raspberry Pi Bookworm
message(VERBOSE, "CMAKE_HOST_SYSTEM_NAME = ${CMAKE_HOST_SYSTEM_NAME}") # Linux on RaspberryPi
message(VERBOSE, "CMAKE_HOST_SYSTEM_PROCESSOR = ${CMAKE_HOST_SYSTEM_PROCESSOR}") # aarch64 on Raspberry Pi
message(VERBOSE, "CMAKE_HOST_SYSTEM_VERSION = ${CMAKE_HOST_SYSTEM_VERSION}") # "6.6.31+rpt-rpi-v8" on Raspberry Pi Bookworm
message(VERBOSE, "CMAKE_SYSTEM = ${CMAKE_SYSTEM}") # "Linux-6.6.31+rpt-rpi-v8" on Raspberry Pi Bookworm
message(VERBOSE, "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}") # Linux on RaspberryPi
message(VERBOSE, "CMAKE_SYSTEM_PROCESSOR = ${CMAKE_SYSTEM_PROCESSOR}") # aarch64 on Raspberry Pi
message(VERBOSE, "CMAKE_SYSTEM_VERSION = ${CMAKE_SYSTEM_VERSION}") # "6.6.31+rpt-rpi-v8" on Raspberry Pi Bookworm
message(VERBOSE, "CMAKE_GENERATOR = ${CMAKE_GENERATOR}") # e.g. Visual Studio 17 2022
message(VERBOSE, "CMAKE_GENERATOR_PLATFORM = ${CMAKE_GENERATOR_PLATFORM}") # x64, win32 on Windows, not set on Linux
message(VERBOSE, "CMAKE_GENERATOR_TOOLSET = ${CMAKE_GENERATOR_TOOLSET}") # e.g. host=x64, not set on Linux
message(VERBOSE, "CMAKE_VS_PLATFORM_NAME = ${CMAKE_VS_PLATFORM_NAME}") # e.g. x64, win32
message(VERBOSE, "CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}") # e.g. 10.0.22621.0
message(VERBOSE, "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(VERBOSE, "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
message(VERBOSE, "CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message(VERBOSE, "CMAKE_CXX_COMPILER_ID = ${CMAKE_CXX_COMPILER_ID}") # "MSVC" for Visual Studio, "GNU" for gcc, "Clang" for Linux Clang, "AppleClang" on Mac
message(VERBOSE, "CMAKE_MODULE_PATH = ${CMAKE_MODULE_PATH}")
message(VERBOSE, "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(CMAKE_SYSTEM MATCHES ".*rpi.*")
	set(RPI ON)
	message(VERBOSE, "Building for Raspberry Pi. Set RPI = ${RPI}")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenGL)
find_package(Freetype REQUIRED)

# Require C++17 to support terse static_assert
# Require C++20 to support designated initializers
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define DEBUG in Debug build, and RELEASE in Release build.
# Don't define either DEBUG or RELEASE in RelWithDebInfo build (CMAKE_CXX_FLAGS_RELWITHDEBINFO) which is essentially a "Dev" build.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

#---------------------------------------------------------------------------------------------------
# HoffGui target
set(HOFFGUI_TARGET "hoffgui")

add_executable(
	${HOFFGUI_TARGET} # name is both name of executable and CMake target created
	MACOSX_BUNDLE # ignored by non-Apple platforms
)

# Would like to call add_subdirectory(src), but can't get source_group(TREE, ...) to work in the subdirectory
# Probably because source_group is not global. "The group is scoped in the directory where the command is called, and applies to 
# sources in targets created in that directory."
set(HOFFGUI_SRC_LIST
	"libs/imgui_markdown/imgui_markdown.h"
	"libs/portable-file-dialogs/portable-file-dialogs.h"
	"libs/tinyxml2/tinyxml2.cpp"
	"libs/tinyxml2/tinyxml2.h"
	"src/Core/Displays.cpp"
	"src/Core/Displays.h"
	"src/Core/Endian.h"
	"src/Core/FileSystem.cpp"
	"src/Core/FileSystem.h"
	"src/Core/FileSystemHelpers.cpp"
	"src/Core/FileSystemHelpers.h"
	"src/Core/Helpers.h"
	"src/Core/hp_assert.cpp"
	"src/Core/hp_assert.h"
	"src/Core/IniFile.cpp"
	"src/Core/IniFile.h"
	"src/Core/Log.cpp"
	"src/Core/Log.h"
	"src/Core/ProcessWrap.cpp"
	"src/Core/ProcessWrap.h"
	"src/Core/StringHelpers.cpp"
	"src/Core/StringHelpers.h"
	"src/Core/Window.cpp"
	"src/Core/Window.h"
	"src/HoffGui/Dialogues/AboutPopup.cpp"
	"src/HoffGui/Dialogues/AboutPopup.h"
	"src/HoffGui/Dialogues/FileDialogue.cpp"
	"src/HoffGui/Dialogues/FileDialogue.h"
	"src/HoffGui/Windows/ImGuiDemoWindow.cpp"
	"src/HoffGui/Windows/ImGuiDemoWindow.h"
	"src/HoffGui/Windows/ModWindow.cpp"
	"src/HoffGui/Windows/ModWindow.h"
	"src/HoffGui/Windows/OptionsWindow.cpp"
	"src/HoffGui/Windows/OptionsWindow.h"
	"src/HoffGui/Windows/OutputWindow.cpp"
	"src/HoffGui/Windows/OutputWindow.h"
	"src/HoffGui/Windows/WindowList.h"
	"src/HoffGui/HoffGui.cpp"
	"src/HoffGui/HoffGui.h"
	"src/HoffGui/Options.cpp"
	"src/HoffGui/Options.h"
	"src/HoffGui/MainMenu.cpp"
	"src/HoffGui/MainMenu.h"
	"src/HoffGui/RecentFiles.cpp"
	"src/HoffGui/RecentFiles.h"
	"src/imgui/backends/imgui_impl_opengl3.cpp"
	"src/imgui/backends/imgui_impl_opengl3.h"
	"src/imgui/backends/imgui_impl_opengl3_loader.h"
	"src/imgui/backends/imgui_impl_sdl2.cpp"
	"src/imgui/backends/imgui_impl_sdl2.h"
	"src/imgui/.editorconfig"
	"src/imgui/imconfig.h"
	"src/imgui/imgui.cpp"
	"src/imgui/imgui.h"
	"src/imgui/imgui.natvis"
	"src/imgui/imgui_demo.cpp"
	"src/imgui/imgui_draw.cpp"
	"src/imgui/imgui_freetype.cpp"
	"src/imgui/imgui_freetype.h"
	"src/imgui/imgui_internal.h"
	"src/imgui/imgui_tables.cpp"
	"src/imgui/imgui_widgets.cpp"
	"src/imgui/imstb_rectpack.h"
	"src/imgui/imstb_textedit.h"
	"src/imgui/imstb_truetype.h"
	"src/ImGuiWrap/Fonts.cpp"
	"src/ImGuiWrap/Fonts.h"
	"src/ImGuiWrap/ImGuiHelpers.cpp"
	"src/ImGuiWrap/ImGuiHelpers.h"
	"src/ImGuiWrap/ImGuiWrap.cpp"
	"src/ImGuiWrap/ImGuiWrap.h"
	"src/Mod/ModFile.cpp"
	"src/Mod/ModFile.h"
	"src/Platform/SystemInfo.cpp"
	"src/Platform/SystemInfo.h"
	"src/Utils/Bitfield.h"
	"src/Utils/Parse.cpp"
	"src/Utils/Parse.h"
	"src/CommandLineArgs.cpp"
	"src/CommandLineArgs.h"
	"src/AppVersion.cpp"
	"src/AppVersion.h"
	"src/main.cpp"
	"CMakeLists.txt"
	"README.md"
	"TODO.md"
)

target_sources(${HOFFGUI_TARGET} PRIVATE ${HOFFGUI_SRC_LIST})

# Runtime resources

# Fonts
set(FONTS_DIR "${PROJECT_SOURCE_DIR}/data/fonts/")
set(FONT_FILES
	"${FONTS_DIR}/proggyfonts/ProggyCleanSZ.ttf"
	"${FONTS_DIR}/proggyfonts/ProggyTinySZ.ttf"
	"${FONTS_DIR}/proggyfonts/ProggyVector-Regular.ttf"
	"${FONTS_DIR}/topaz-unicode/topaz_unicode_ks13_regular.ttf"
)

# Package text files
set(HOFFGUI_DOCS_DIR "${PROJECT_SOURCE_DIR}/docs")
set(PACKAGE_TEXT_FILES
	"${HOFFGUI_DOCS_DIR}/README.txt"
	"${PROJECT_SOURCE_DIR}/LICENSE.txt"
	"${PROJECT_SOURCE_DIR}/VERSION.txt"
	"${HOFFGUI_DOCS_DIR}/CHANGELOG.txt"
	"${HOFFGUI_DOCS_DIR}/CONTRIBUTING.md"
	"${HOFFGUI_DOCS_DIR}/FAQ.txt"
)

if(APPLE)
	# Use custom Info.plist so can set properties not supported by default template MacOSXBundleInfo.plist.in
	# Especially NSHighResolutionCapable
#	set(MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/packaging/MacOSXBundleInfo.plist.in) # Doesn't work
	set_target_properties(${HOFFGUI_TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/packaging/MacOSXBundleInfo.plist.in)
	set(MACOSX_BUNDLE_BUNDLE_NAME "hoffgui")
	set(MACOSX_BUNDLE_BUNDLE_VERSION ${hoffgui_VERSION})
	set(MACOSX_BUNDLE_LONG_VERSION_STRING ${hoffgui_VERSION})
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${hoffgui_VERSION})
	set(MACOSX_BUNDLE_COPYRIGHT "© 2024, Howard Price")
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "org.howprice.hoffgui") # CFBundleIdentifier

	# NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
	# the property added to Info.plist
	set(MACOSX_BUNDLE_ICON_FILE icon.icns)
	# Tell CMake where to find and install the icon file itself, and other resources
	set(HOFFGUI_BUNDLE_RESOURCES 
		${PROJECT_SOURCE_DIR}/packaging/icon.icns
		${TOOL_EXES}
	)
	set_source_files_properties(${HOFFGUI_BUNDLE_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
	set_source_files_properties(${FONT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/fonts")
	set_source_files_properties(${PACKAGE_TEXT_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "SharedSupport")

	#set(MACOSX_BUNDLE_INFO_STRING "© 2024, Howard Price") # Obsolete; CFBundleGetInfoString superceded by NSHumanReadableCopyright (NSHumanReadableCopyright)

	target_sources(
		${HOFFGUI_TARGET} 
		PRIVATE 
		${HOFFGUI_BUNDLE_RESOURCES}
		${FONT_FILES}
		${PACKAGE_TEXT_FILES}
	)

endif()

set(ICON_RESOURCE "")
if(WIN32)
    set(ICON_RESOURCE "${PROJECT_SOURCE_DIR}/packaging/hoffgui.icon.rc")
elseif (APPLE) #osx icon resource file, provided from /packaging directory
    #set(ICON_RESOURCE "${PROJECT_SOURCE_DIR}/packaging/exampleApp.icns")
endif()

target_sources(${HOFFGUI_TARGET} PRIVATE ${ICON_RESOURCE})

set_property(TARGET ${HOFFGUI_TARGET} PROPERTY COMPILE_WARNING_AS_ERROR ON)

# Compiler-specific options
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

	target_compile_options(
		${HOFFGUI_TARGET} 
		PRIVATE
		/W4 
		/wd4127 # conditional expression is constant
		/we4062 # enumerator 'identifier' in switch of enum 'enumeration' is not handled
	)

	# Allow use of portable fopen (fopen_s is Windows only)
	target_compile_definitions(${HOFFGUI_TARGET} PUBLIC _CRT_SECURE_NO_WARNINGS _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # GCC

	target_compile_options(
		${HOFFGUI_TARGET} 
		PRIVATE 
		-Wall 
		-Wextra 

		# Unfortunately seem to have to disable -pendantic to use anonymous structs (68000 StatusRegister) :-(
		# -pedantic
		
		-Wno-unknown-pragmas # Can't do in source file. See https://stackoverflow.com/questions/12842306/suppress-wunknown-pragmas-warning-in-gcc 
		-Werror

		-Wno-unused-const-variable

		# TODO: Try to anable Microsoft extensions and reinstate -pedantic above
		# Suppress error: ISO C++ prohibits anonymous structs [-Werror=pedantic] for 68000 StatusRegister union
		#-fms-extensions
	)

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	
	target_compile_options(
		${HOFFGUI_TARGET} 
		PRIVATE 
		-Wall 
		-Wextra 
		-pedantic
		-Wno-unknown-pragmas # Can't do in source file. See https://stackoverflow.com/questions/12842306/suppress-wunknown-pragmas-warning-in-gcc 
		-Werror

		-Wno-unused-const-variable

		# Suppress error: ISO C++ prohibits anonymous structs [-Werror=pedantic] for 68000 StatusRegister union
		-Wno-gnu-anonymous-struct -Wno-nested-anon-types
	)
	
else()
	message(FATAL_ERROR, "Unknown compiler")
endif()

# MSVC multi-processor Compilation. Can reduce build time dramatically.
target_compile_options(${HOFFGUI_TARGET} PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MP>)

# Enable crash (assert/fatal) logging to file
target_compile_definitions(${HOFFGUI_TARGET} PRIVATE HP_WRITE_CRASH_LOG=1)

target_include_directories(
	${HOFFGUI_TARGET} 
	PRIVATE
	"src"
	"libs/portable-file-dialogs"
	"libs/NDK/Include_H/"
	"libs/tinyxml2/"
	"libs/imgui_markdown/"
)

target_link_libraries(${HOFFGUI_TARGET} PRIVATE SDL2::SDL2)
target_link_libraries(${HOFFGUI_TARGET} PRIVATE SDL2::SDL2main)
target_link_libraries(${HOFFGUI_TARGET} PRIVATE Freetype::Freetype) # since CMake 3.10

# link opengl32
target_link_libraries(${HOFFGUI_TARGET} PRIVATE OpenGL::GL)

# Define grouping for source files in IDE project generation. This is just for Visual Studio IDE convenience.
# Without this, all .cpp files are in the "Source Files" filter, and all .h files are in the "Header files" filter.
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HOFFGUI_SRC_LIST}) # Requires CMake 3.8

# Make the Windows application DPI aware
# This means that the OS does not do any upscaling for Desktop Scaling settings > 100%, so the application
# has to handle it all.
# Set the Manifest Tool -> Input and Output -> DPI Awareness in the Visual Studio target project properties.
# Currently disabled pending application changes to support this e.g. ImGui Font size and font size relative layouts
set_property(TARGET ${HOFFGUI_TARGET} PROPERTY VS_DPI_AWARE "PerMonitor") # PerMonitor ON OFF

if(NOT APPLE) # Not required on macOS because already included in Bundle

	# Debug print TARGET_FILE_DIR n.b. Generator expressions can only be displayed at *build* time.
	add_custom_command(TARGET ${HOFFGUI_TARGET} POST_BUILD
  		COMMAND ${CMAKE_COMMAND} -E echo 
  		"TARGET_FILE_DIR = $<TARGET_FILE_DIR:${HOFFGUI_TARGET}>")

	# Fonts
	add_custom_command(TARGET ${HOFFGUI_TARGET} POST_BUILD COMMAND
		${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/fonts)
	add_custom_command(TARGET ${HOFFGUI_TARGET} POST_BUILD COMMAND
		${CMAKE_COMMAND} -E copy_if_different ${FONT_FILES} $<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/fonts)

	# Copy package text files next to exe (cpack generators do not take care of this)
	add_custom_command(TARGET ${HOFFGUI_TARGET} POST_BUILD COMMAND
		${CMAKE_COMMAND} -E copy_if_different ${PACKAGE_TEXT_FILES} $<TARGET_FILE_DIR:${HOFFGUI_TARGET}>)
endif()

# Default Visual Studio start up project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${HOFFGUI_TARGET})

set_target_properties(
	${HOFFGUI_TARGET} PROPERTIES
#	VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
	VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/data
)

# ----------------------------------------------------------------------------
# Install

# TODO Try GNUInstallDirs
# include(GNUInstallDirs)

#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Default install prefix" FORCE)
#endif()

# CMake 3.14 can use default destinations to install targets, but I don't want the executable in bin/

install(TARGETS ${HOFFGUI_TARGET}
	RUNTIME DESTINATION .
	BUNDLE DESTINATION .
)

if(WIN32)

	# Install SDL2.dll next to the executable
	# This is no longer required now that TARGET_RUNTIME_DLLS is used below
#	install(FILES 
#		$<TARGET_FILE:SDL2::SDL2>
#		DESTINATION .
#	)

	# Install both SDL2.dll and freetype.dll next to the executable
	# However, this does not install the dependencies of freetype.dll: zlib1.dll, libpng16.dll, brotlicommon.dll, brotlidec.dll, bz2.dll
	install(FILES 
		$<TARGET_RUNTIME_DLLS:${HOFFGUI_TARGET}>
		DESTINATION .
	)

	# Install freetype.dll dependencies manually
	# I tried using file(GET_RUNTIME_DEPENDENCIES but didn't work
	set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE) # Required to make CPack verbose
	message(STATUS "TARGET_FILE_DIR: $<TARGET_FILE_DIR:${HOFFGUI_TARGET}") # Prints at install time i.e. CPack n.b. Must set vscode settings "Cpack args" and add --verbose
	install(FILES 
		$<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/brotlicommon.dll 
		$<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/brotlidec.dll 
		$<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/bz2.dll 
		$<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/libpng16.dll 
		$<TARGET_FILE_DIR:${HOFFGUI_TARGET}>/zlib1.dll 
		DESTINATION .
	)

	# #TODO: Install SDL2.dll and freetype.dll and all of the recursive dependencies
	# Try using file(GET_RUNTIME_DEPENDENCIES
	# See:
	# - Docs: https://cmake.org/cmake/help/latest/command/file.html#handling-runtime-binaries
	# - Example: https://github.com/msclock/cmake-modules/blob/master/cmake/install/InstallDependency.cmake
	# - https://github.com/TrenchBroom/TrenchBroom/issues/3996#issuecomment-1100737969
	# - https://github.com/microsoft/vcpkg/issues/13797#issuecomment-700605701
	# Transfer the value of ${HOFFGUI_TARGET} into the install script
#	install(CODE "set(HOFFGUI_EXECUTABLE \"$<TARGET_FILE:${HOFFGUI_TARGET}>\")")
#	
#	install(CODE [[
#		
#		message(STATUS "EXECUTABLES: ${HOFFGUI_EXECUTABLE}") # Prints at install time i.e. CPack n.b. Must set vscode settings "Cpack args" and add --verbose
#		
#		file(GET_RUNTIME_DEPENDENCIES
#			EXECUTABLES ${HOFFGUI_EXECUTABLE}
#			RESOLVED_DEPENDENCIES_VAR _r_deps
#			UNRESOLVED_DEPENDENCIES_VAR _u_deps
#			CONFLICTING_DEPENDENCIES_PREFIX _c_deps
#		)
#
#		#if(_r_deps)
#			message(STATUS "Resolved dependencies: ${_r_deps}")
#		#endif()
#
#		if(_u_deps)
#			message(STATUS "Unresolved dependencies: ${_u_deps}")
#		endif()
#
#		if(_c_deps)
#			message(STATUS "Conflict dependencies: ${_c_deps_FILENAMES}")
#		endif()
#	]])

endif()

if(NOT APPLE) # These files are already included in the macOS bundle

	# ira and vasm executables
	install(FILES ${TOOL_EXES}
		DESTINATION .
		PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	install(FILES ${FONT_FILES}
		DESTINATION "fonts")

	# README, LICENSE, etc.
	install(FILES ${PACKAGE_TEXT_FILES}
		DESTINATION .)
		
endif()

# ----------------------------------------------------------------------------
# Package

# Ref: https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html

# On Windows, this adds the Visual C++ Redistributable DLLs to the package's bin directory
# I don't want to do this for now; Users can install the Visual C++ Redistributable themselves.
# include(InstallRequiredSystemLibraries)

# These are project-specific
set(CPACK_PACKAGE_NAME hoffgui)

set(CPACK_PACKAGE_VENDOR howprice)
set(CPACK_PACKAGE_CONTACT "h0ffman <h0ffman@h0ffman.com>") # Required for CPackDeb: Debian package
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
	"Hoff GUI")
set(HOMEPAGE "https://www.twitch.tv/djh0ffman")

# These files get embedded in installers, but not in the package itself
# set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/package/description.txt")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/docs/README.txt")

# Displayed by Mac DragNDrop (DMG) package generator
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})

# Built packages can be found in the repo root packages/ directory
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_SOURCE_DIR}/packages")

set(CPACK_STRIP_FILES TRUE) # Strip debug symbols?
set(CPACK_VERBATIM_VARIABLES TRUE)

# n.b. No need to set CPACK_PACKAGE_VERSION because use VERSION keyword in the project() call

# The cpack -G option overrides any list set by the project with the CPACK_GENERATOR variable, so the
# command line still has full control.
if(WIN32)
	set(CPACK_GENERATOR ZIP)
elseif(APPLE)
	set(CPACK_GENERATOR 
		DragNDrop
		TGZ 
		#productbuild #TODO: What is this for? App Store package?
	)

	# CPack DragNDrop generator variables
	# https://cmake.org/cmake/help/latest/cpack_gen/dmg.html
	set(CPACK_DMG_VOLUME_NAME "hoffgui")
	set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${PROJECT_SOURCE_DIR}/packaging/DS_Store.scpt")
	set(CPACK_DMG_BACKGROUND_IMAGE "${PROJECT_SOURCE_DIR}/packaging/dmg_background.png")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CPACK_GENERATOR TGZ DEB)

	# TODO: Add OpenGL?
	# TODO: How to determine dependencies?
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0") # Comma seperated list
else()
	set(CPACK_GENERATOR TGZ)
endif()


# Override the default CPACK_PACKAGE_FILE_NAME = ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME} e.g. hoffgui-0.1.0-win64
# This generates a zip with this name, containing a folder with this name.
# I want a consistent package name, without dots, so I can upload a consistent package filename to itch.io
# This is compounded by a bug in ira which doesn't like dots in the path!
#
# Must override CPACK_PACKAGE_FILE_NAME before include(CPack)
# n.b. Default CPACK_SYSTEM_NAME is set during include(CPack), so need to set manually here from CMAKE_SYSTEM_NAME taking into account 32/64 Windows 

# Logic copied from CPack.cmake
set(system_name ${CMAKE_SYSTEM_NAME})
if(system_name MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(system_name win64)
  else()
    set(system_name win32)
  endif()
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${system_name}") # removed version number for consitent package name and structure between versions
message(VERBOSE, "CPACK_PACKAGE_FILE_NAME = ${CPACK_PACKAGE_FILE_NAME}") # set during call to include(CPack)

# This is what writes out the input file for cpack
include(CPack) 
