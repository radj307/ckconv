# ckconv/ckconv
cmake_minimum_required (VERSION 3.20)

file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)
file(GLOB SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/rc")

file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/rc/version.h")
MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/rc/version.h" "ckconv" "${ckconv_VERSION_EXTENDED}")

if (WIN32)
	set(ckconv_rc "${CMAKE_CURRENT_SOURCE_DIR}/rc/ckconv.rc")
	include(ResourceMaker)

	MAKE_STRINGRC_ICON(ckconv_stringrc_icon "${CMAKE_CURRENT_SOURCE_DIR}/rc/ckconv.ico")
	MAKE_STRINGRC_VERSIONINFO(ckconv_stringrc_versioninfo
		"${ckconv_VERSION}"
		"GNU General Public License v3 @ radj307"
		"radj307"
		"ckconv"
		"Commandline unit converter for Bethesda games."
	)

	MAKE_RESOURCE("${ckconv_rc}" "${ckconv_stringrc_icon}" "${ckconv_stringrc_versioninfo}")
endif()

add_executable (ckconv "${SRCS}" "${ckconv_rc}" "rc/version.h")

set_property(TARGET ckconv PROPERTY CXX_STANDARD 23)
set_property(TARGET ckconv PROPERTY CXX_STANDARD_REQUIRED ON)

if (MSVC)
	target_compile_options(ckconv PRIVATE "/Zc:__cplusplus" "/Zc:preprocessor" "/permissive-")
endif()

target_sources(ckconv PRIVATE "${HEADERS}")

target_link_libraries(ckconv PRIVATE TermAPI filelib)

option(ckconv_DISABLE_CONFIG_FILE "Don't enable the code for the INI configuration file." FALSE)
if (NOT ${ckconv_DISABLE_CONFIG_FILE})
	target_compile_definitions(ckconv PRIVATE ENABLE_CONFIG_FILE)
endif()

if (${307lib_build_netlib})
	include(FetchContent)
	FetchContent_Declare(nlohmann_json
		GIT_REPOSITORY https://github.com/nlohmann/json.git
		GIT_TAG v3.11.2
	)
	FetchContent_Declare(cpr
		GIT_REPOSITORY https://github.com/libcpr/cpr.git
		GIT_TAG 1.9.1
	)
	FetchContent_MakeAvailable(nlohmann_json cpr)

	target_link_libraries(ckconv PRIVATE nlohmann_json::nlohmann_json cpr::cpr)
	target_compile_definitions(ckconv PRIVATE ENABLE_UPDATE_CHECK)
endif()

include(PackageInstaller)
INSTALL_EXECUTABLE(ckconv "${CMAKE_INSTALL_PREFIX}")
