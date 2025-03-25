if (NOT USE_CESIUM_OLD_BUILD)

# Use vckpg

cmake_minimum_required(VERSION 3.15)

set(CESIUM_TESTS_ENABLED False)

if (NOT VCPKG_LIBRARY_LINKAGE)
    set(VCPKG_LIBRARY_LINKAGE static)
endif()

if(NOT VCPKG_MANIFEST_MODE)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ezvcpkg/ezvcpkg.cmake)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/detect-vcpkg-triplet.cmake)

if (NOT VCPKG_TRIPLET)
    if (DEFINED ENV{VCPKG_TRIPLET})
        set(VCPKG_TRIPLET "$ENV{VCPKG_TRIPLET}")
    elseif(DETECTED_VCPKG_TRIPLET_ERROR)
        message(FATAL_ERROR "${DETECTED_VCPKG_TRIPLET_ERROR}")
    elseif(DETECTED_VCPKG_TRIPLET STREQUAL "x64-windows")
        # cesium-native requires static linking on Windows
        set(VCPKG_TRIPLET "x64-windows-static-md")
    else()
        set(VCPKG_TRIPLET "${DETECTED_VCPKG_TRIPLET}")
    endif()
endif()

message(STATUS "VCPKG_TRIPLET ${VCPKG_TRIPLET}")

if (NOT VCPKG_OVERLAY_PORTS)
    if (DEFINED ENV{VCPKG_OVERLAY_PORTS})
        set(VCPKG_OVERLAY_PORTS "$ENV{VCPKG_OVERLAY_PORTS}")
    endif()
endif()

list(APPEND VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/ports")

message(STATUS "VCPKG_OVERLAY_PORTS ${VCPKG_OVERLAY_PORTS}")

if (NOT VCPKG_OVERLAY_TRIPLETS)
    if (DEFINED ENV{VCPKG_OVERLAY_TRIPLETS})
        set(VCPKG_OVERLAY_TRIPLETS "$ENV{VCPKG_OVERLAY_TRIPLETS}")
    endif()
endif()

message(STATUS "VCPKG_OVERLAY_TRIPLETS ${VCPKG_OVERLAY_TRIPLETS}")

# These packages are used in the public headers of Cesium libraries, so we need to distribute the headers and binaries
# with the installation
# Note that fmt is a public dependency of the vcpkg version of spdlog
# STB is not technically part of the public interface, but it is used by the downstream Cesium for Unreal project
set(PACKAGES_PUBLIC asyncplusplus expected-lite fmt glm rapidjson spdlog stb ada-url)
# These packages are used in the code and produce binaries, but are not part of the public interface.  Therefore we need
# to distribute the binaries for linking, but not the headers, as downstream consumers don't need them
# OpenSSL and abseil are both dependencies of s2geometry
set(PACKAGES_PRIVATE
    abseil draco ktx modp-base64 meshoptimizer openssl s2geometry
    libjpeg-turbo sqlite3 tinyxml2 libwebp zlib-ng picosha2
    earcut-hpp cpp-httplib[core] libmorton zstd
)

# Packages only used for testing
set(PACKAGES_TEST doctest)

if(NOT VCPKG_MANIFEST_MODE)
    set(PACKAGES_ALL ${PACKAGES_PUBLIC})
    list(APPEND PACKAGES_ALL ${PACKAGES_PRIVATE})
    list(APPEND PACKAGES_ALL ${PACKAGES_TEST})

    ezvcpkg_fetch(
        COMMIT 2024.11.16
        PACKAGES ${PACKAGES_ALL}
        # Clean the build trees after building, so that we don't use a ton a disk space on the CI cache
        CLEAN_BUILDTREES
        # Update the cmake toolchain so it can find the above packages
        UPDATE_TOOLCHAIN
        # Force the installation of each package one at a time, or the Travis CI build will time out waiting for output
        SERIALIZE
    )
endif()

if (NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Specify the VCPKG toolchain on the command line as '-DCMAKE_TOOLCHAIN_FILE=<VCPKG_ROOT>/scripts/buildsystems/vcpkg.cmake'")
else()
    message(STATUS "CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Set defaults that should be set BEFORE compiler / IDE detection
include("cmake/defaults.cmake")

project(cesium-native
    VERSION 0.44.3
    LANGUAGES CXX C
)

include(GNUInstallDirs)
include(CMakeDependentOption)

set(PACKAGE_BASE_DIR "${EZVCPKG_PACKAGES_DIR}")
set(PACKAGE_BUILD_DIR "${EZVCPKG_DIR}")

if(VCPKG_MANIFEST_MODE)
    set(PACKAGE_BUILD_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TRIPLET}/")
    set(PACKAGE_BASE_DIR "$ENV{VCPKG_ROOT}/packages")
endif()

message(STATUS "PACKAGE_BASE_DIR ${PACKAGE_BASE_DIR}")
message(STATUS "PACKAGE_BUILD_DIR ${PACKAGE_BUILD_DIR}")


option(CESIUM_INSTALL_STATIC_LIBS "Whether to install the static libraries of cesium-native and its dependencies." ON)
option(CESIUM_INSTALL_HEADERS "Whether to install the header files of cesium-native and its public dependencies." ON)
option(CESIUM_ENABLE_CLANG_TIDY "Enable clang-tidy targets for static code analysis." ON)

cmake_dependent_option(
    CESIUM_ENABLE_CLANG_TIDY_ON_BUILD
    "Run clang-tidy while building. Will slow down the build process. Available only if CESIUM_ENABLE_CLANG_TIDY is ON."
    OFF
    CESIUM_ENABLE_CLANG_TIDY
    OFF
)

set(CESIUM_CLANG_TIDY_USE_THREADS 14 CACHE STRING "Sets the number of threads for run-clang-tidy to use.")

if(CESIUM_INSTALL_STATIC_LIBS OR CESIUM_INSTALL_HEADERS)
    foreach(PACKAGE ${PACKAGES_PUBLIC})
        string(REGEX REPLACE "\[.*\]" "" PACKAGE ${PACKAGE})
        set(PACKAGE_DIR ${PACKAGE_BASE_DIR}/${PACKAGE}_${VCPKG_TRIPLET})
        message(DEBUG "PACKAGE_DIR ${PACKAGE_DIR}")

        if(CESIUM_INSTALL_HEADERS AND NOT PACKAGE IN_LIST CESIUM_EXCLUDE_INSTALL_HEADERS)
            install(
                DIRECTORY ${PACKAGE_DIR}/include/
                TYPE INCLUDE
            )
        endif()

        if (CESIUM_INSTALL_STATIC_LIBS AND NOT PACKAGE IN_LIST CESIUM_EXCLUDE_INSTALL_STATIC_LIBS AND EXISTS ${PACKAGE_DIR}/lib)
            install(
                DIRECTORY $<IF:$<CONFIG:Debug>,${PACKAGE_DIR}/debug/lib/,${PACKAGE_DIR}/lib/>
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
    endforeach()
endif()

if(CESIUM_INSTALL_STATIC_LIBS)
    foreach(PACKAGE ${PACKAGES_PRIVATE})
        set(PACKAGE_DIR ${PACKAGE_BASE_DIR}/${PACKAGE}_${VCPKG_TRIPLET})
        message(DEBUG "PACKAGE_DIR ${PACKAGE_DIR}")
        if (NOT PACKAGE IN_LIST CESIUM_EXCLUDE_INSTALL_STATIC_LIBS AND EXISTS ${PACKAGE_DIR}/lib)
            install(
                DIRECTORY $<IF:$<CONFIG:Debug>,${PACKAGE_DIR}/debug/lib/,${PACKAGE_DIR}/lib/>
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif()
    endforeach()
endif()

if(NOT DEFINED CMAKE_C_COMPILER_LAUNCHER AND NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_FOUND ccache)
    find_program(SCCACHE_FOUND sccache)
    if (SCCACHE_FOUND)
        message("setting SCCACHE to ${SCCACHE_FOUND}")
        set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
    elseif(CCACHE_FOUND)
        message("setting CCACHE to ${CCACHE_FOUND}")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_FOUND})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
    endif()
endif()

# ccache/sccache only works with /Z7, not /Zi, so tweak the Debug and RelWithDebInfo build flags in the presence of a compiler cache
if(MSVC AND (DEFINED CMAKE_C_COMPILER_LAUNCHER))
    message(DEBUG "Setting MSVC flags to /Z7 for ccache compatibility.  Current flags: ${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "/Z7" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    string(REPLACE "/Zi" "/Z7" CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    message(DEBUG "New flags: ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

option(CESIUM_TRACING_ENABLED "Whether to enable the Cesium performance tracing framework (CESIUM_TRACE_* macros)." OFF)
option(CESIUM_COVERAGE_ENABLED "Whether to enable code coverage" OFF)
option(CESIUM_TESTS_ENABLED "Whether to enable tests" ON)
option(CESIUM_GLM_STRICT_ENABLED "Whether to force strict GLM compile definitions." ON)
option(CESIUM_DISABLE_DEFAULT_ELLIPSOID "Whether to disable the WGS84 default value for ellipsoid parameters across cesium-native." OFF)
option(CESIUM_MSVC_STATIC_RUNTIME_ENABLED "Whether to enable static linking for MSVC runtimes" OFF)
option(CESIUM_DEBUG_TILE_UNLOADING "Whether to enable tracking of tile _doNotUnloadSubtreeCount modifications for tile unloading debugging." OFF)

if (CESIUM_MSVC_STATIC_RUNTIME_ENABLED)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if (CESIUM_TRACING_ENABLED)
    add_compile_definitions(CESIUM_TRACING_ENABLED=1)
endif()

# Set defaults that need to be set AFTER compiler / IDE detection
include("cmake/compiler.cmake")

if(CESIUM_ENABLE_CLANG_TIDY)
    setup_clang_tidy(
        PROJECT_BUILD_DIRECTORY
        "${PROJECT_BINARY_DIR}"
        PROJECT_SOURCE_DIRECTORIES
        "${PROJECT_SOURCE_DIR}"
        ENABLE_CLANG_TIDY_ON_BUILD
        ${CESIUM_ENABLE_CLANG_TIDY_ON_BUILD}
    )
endif()

# Add Modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/extern/cmake-modules/")
if (CESIUM_COVERAGE_ENABLED AND NOT MSVC)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_html(
        NAME cesium-native-tests-coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        EXCLUDE "${PROJECT_SOURCE_DIR}/extern/*" "${PROJECT_BINARY_DIR}"
        DEPENDENCIES cesium-native-tests
    )
endif()

if (NOT DEFINED GLOB_USE_CONFIGURE_DEPENDS)
    set(GLOB_USE_CONFIGURE_DEPENDS OFF CACHE BOOL
        "Controls if cesium-native targets should use configure_depends or not for globbing their sources"
    )
endif()

# On the CI builds, I have to do this explicitly for some reason or it fails to find the vcpkg packages.
# The toolchain is supposed to manage this, but I haven't figured out why it isn't yet.

list(APPEND CMAKE_PREFIX_PATH "${PACKAGE_BUILD_DIR}/share/s2")
list(APPEND CMAKE_PREFIX_PATH "${PACKAGE_BUILD_DIR}/share")
list(APPEND CMAKE_PREFIX_PATH "${PACKAGE_BUILD_DIR}")

# Find the VCPKG dependnecies
# Note that while we could push these into the extern/CMakeLists.txt as an organization tidy-up, that would require
# us to update the minimum version of CMake to 3.24 and to add the GLOBAL option to the find_package calls, otherwise
# they won't be visible in this scope nor any of the subdirectories for the actual libraries.
#
# However, for some of the vcpkg built libraries where they don't provide a prope cmake config file, we have to declare
# and imporeted library target ourselves. This is the case for modp_b64::modp_b64, picosha2::picosha2 and earcut. In
# these cases, we *do* have the somewhat ugly and verbose details in the extern/CMakeLists.txt file.
find_package(Async++ CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)
find_package(draco CONFIG REQUIRED)
find_package(expected-lite CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(libmorton CONFIG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(s2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(WebP CONFIG REQUIRED)
find_package(ada CONFIG REQUIRED)

add_subdirectory(extern EXCLUDE_FROM_ALL)

# Public Targets
add_subdirectory(CesiumUtility)
add_subdirectory(CesiumGltf)
add_subdirectory(CesiumGeometry)
add_subdirectory(CesiumGeospatial)
add_subdirectory(CesiumJsonReader)
add_subdirectory(CesiumJsonWriter)
add_subdirectory(CesiumGltfContent)
add_subdirectory(CesiumGltfReader)
add_subdirectory(CesiumGltfWriter)
add_subdirectory(CesiumAsync)
add_subdirectory(Cesium3DTiles)
add_subdirectory(Cesium3DTilesReader)
add_subdirectory(Cesium3DTilesWriter)
add_subdirectory(Cesium3DTilesContent)
add_subdirectory(CesiumRasterOverlays)
add_subdirectory(Cesium3DTilesSelection)
add_subdirectory(CesiumIonClient)
add_subdirectory(CesiumQuantizedMeshTerrain)

# Private Targets
if (CESIUM_TESTS_ENABLED)
    # enable_testing() MUST be called before add_subdirectory or no tests
    # will be found by ctest
    enable_testing()
    add_subdirectory(CesiumNativeTests)
endif()


add_subdirectory(doc)

#####################################################################################
#####################################################################################
#####################################################################################
#####################################################################################
#####################################################################################

else() # USE_CESIUM_OLD_BUILD

set(CESIUM_TESTS_ENABLED False)

# Set defaults that should be set BEFORE compiler / IDE detection
include("cmake/defaults.cmake")

cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cesium-native
    VERSION 0.44.2
    LANGUAGES CXX C
)

include(GNUInstallDirs)
include(CMakeDependentOption)

option(PRIVATE_CESIUM_SQLITE "ON to rename SQLite symbols to cesium_sqlite3_* so they won't conflict with other SQLite implemenentations" OFF)
option(CESIUM_TRACING_ENABLED "Whether to enable the Cesium performance tracing framework (CESIUM_TRACE_* macros)." OFF)
option(CESIUM_COVERAGE_ENABLED "Whether to enable code coverage" OFF)
option(CESIUM_TESTS_ENABLED "Whether to enable tests" ON)
option(CESIUM_GLM_STRICT_ENABLED "Whether to force strict GLM compile definitions." ON)

if (CESIUM_TRACING_ENABLED)
    add_compile_definitions(CESIUM_TRACING_ENABLED=1)
endif()

# Add Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/extern/cmake-modules/")

if (CESIUM_COVERAGE_ENABLED AND NOT MSVC)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_html(
        NAME cesium-native-tests-coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        EXCLUDE "${PROJECT_SOURCE_DIR}/extern/*" "${PROJECT_BINARY_DIR}"
        DEPENDENCIES cesium-native-tests
    )
endif()

if (NOT DEFINED GLOB_USE_CONFIGURE_DEPENDS)
    set(GLOB_USE_CONFIGURE_DEPENDS OFF CACHE BOOL
        "Controls if cesium-native targets should use configure_depends or not for globbing their sources"
    )
endif()

set(CESIUM_DEBUG_POSTFIX "d")
set(CESIUM_RELEASE_POSTFIX "")

set(CMAKE_DEBUG_POSTFIX ${CESIUM_DEBUG_POSTFIX})
set(CMAKE_RELEASE_POSTFIX ${CESIUM_RELEASE_POSTFIX})
set(CMAKE_MINSIZEREL_POSTFIX ${CESIUM_RELEASE_POSTFIX})
set(CMAKE_RELWITHDEBINFO_POSTFIX ${CESIUM_RELEASE_POSTFIX})

# Use configure_depends to automatically reconfigure on filesystem
# changes at the expense of computational overhead for CMake to
# determine if new files have been added (-DGLOB_USE_CONFIGURE_DEPENDS).

function(cesium_glob_files out_var_name regexes)
    set(files "")
    foreach(arg ${ARGV})
        list(APPEND regexes_only "${arg}")
    endforeach()
    list(POP_FRONT regexes_only)
    if (GLOB_USE_CONFIGURE_DEPENDS)
        file(GLOB_RECURSE files CONFIGURE_DEPENDS ${regexes_only})
    else()
        file(GLOB files ${regexes_only})
    endif()
    set(${ARGV0} "${files}" PARENT_SCOPE)
endfunction()

# Workaround for targets that erroneously forget to
# declare their include directories as `SYSTEM`
function(target_link_libraries_system target scope)
  set(libs ${ARGN})
  foreach(lib ${libs})
    get_target_property(lib_include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)

    if ("${lib_include_dirs}" MATCHES ".*NOTFOUND$")
        message(FATAL_ERROR "${target}: Cannot use INTERFACE_INCLUDE_DIRECTORIES from target ${lib} as it does not define it")
    endif()

    target_include_directories(${target} SYSTEM ${scope} ${lib_include_dirs})
    target_link_libraries(${target} ${scope} ${lib})
  endforeach()
endfunction()

# Shared object support is currently NOT working on Windows
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

function(configure_cesium_library targetName)
    if (MSVC)
        target_compile_options(${targetName} PRIVATE /W4 /WX /wd4201 /bigobj)
    else()
        target_compile_options(${targetName} PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wshadow -Wsign-conversion)
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13)
        # Disable dangling-reference warning due to amount of false positives: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109642
        target_compile_options(${targetName} PRIVATE -Wno-dangling-reference)
    endif()

    set_target_properties(${targetName} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

    if (CESIUM_GLM_STRICT_ENABLED)
        target_compile_definitions(
            ${targetName}
            PUBLIC
                GLM_FORCE_XYZW_ONLY # Disable .rgba and .stpq to make it easier to view values from debugger
                GLM_FORCE_EXPLICIT_CTOR # Disallow implicit conversions between dvec3 <-> dvec4, dvec3 <-> fvec3, etc
                GLM_FORCE_SIZE_T_LENGTH # Make vec.length() and vec[idx] use size_t instead of int
        )
    endif()

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(
            ${targetName}
            PUBLIC
                CESIUM_SHARED=${BUILD_SHARED_LIBS}
        )
    endif()

    if (NOT ${targetName} MATCHES "cesium-native-tests")
        string(TOUPPER ${targetName} capitalizedTargetName)
        target_compile_definitions(
            ${targetName}
            PRIVATE
                ${capitalizedTargetName}_BUILDING
        )
    endif()

    if (CESIUM_EXTRA_INCLUDES)
      target_include_directories(${targetName} PRIVATE ${CESIUM_EXTRA_INCLUDES})
    endif()

endfunction()


# Private Libraries
add_subdirectory(extern EXCLUDE_FROM_ALL)

# These libraries override the debug postfix, so re-override it.
set_target_properties(spdlog PROPERTIES DEBUG_POSTFIX ${CESIUM_DEBUG_POSTFIX})
set_target_properties(tinyxml2 PROPERTIES DEBUG_POSTFIX ${CESIUM_DEBUG_POSTFIX})

add_library(s2::s2 ALIAS s2geometry)
add_library(draco::draco ALIAS ${CESIUM_NATIVE_DRACO_LIBRARY})
add_library(meshoptimizer::meshoptimizer ALIAS meshoptimizer)
add_library(modp_b64::modp_b64 ALIAS modp_b64)
add_library(KTX::ktx ALIAS ktx)
add_library(WebP::webp ALIAS webp)
add_library(WebP::webpdecoder ALIAS webpdecoder)
add_library(libjpeg-turbo::turbojpeg ALIAS turbojpeg)
add_library(libjpeg-turbo::turbojpeg-static ALIAS turbojpeg)
add_library(unofficial::sqlite3::sqlite3 ALIAS sqlite3)
add_library(tinyxml2::tinyxml2 ALIAS tinyxml2)
add_library(picosha2::picosha2 ALIAS PicoSHA2)
add_library(zlib-ng::zlib-ng ALIAS zlib)
add_library(ada::ada ALIAS ada)
add_library(spdlog::spdlog ALIAS spdlog)
add_library(spdlog::spdlog_header_only ALIAS spdlog)

# Add global headers includes needed for several Modules
include_directories(SYSTEM 
    ${CESIUM_NATIVE_RAPIDJSON_INCLUDE_DIR}
    ${CESIUM_NATIVE_DRACO_INCLUDE_DIR}
    ${CESIUM_NATIVE_STB_INCLUDE_DIR}
    ${CESIUM_NATIVE_LIBMORTON_INCUDE_DIR}
    ${CESIUM_NATIVE_FMT_INCUDE_DIR}
    ${CESIUM_NATIVE_SQLITE3_INCUDE_DIR}
)

# Public Targets
add_subdirectory(CesiumUtility)
add_subdirectory(CesiumGltf)
add_subdirectory(CesiumGeometry)
add_subdirectory(CesiumGeospatial)
add_subdirectory(CesiumJsonReader)
add_subdirectory(CesiumJsonWriter)
add_subdirectory(CesiumGltfContent)
add_subdirectory(CesiumGltfReader)
add_subdirectory(CesiumGltfWriter)
add_subdirectory(CesiumAsync)
add_subdirectory(Cesium3DTiles)
add_subdirectory(Cesium3DTilesReader)
add_subdirectory(Cesium3DTilesWriter)
add_subdirectory(Cesium3DTilesContent)
add_subdirectory(CesiumRasterOverlays)
add_subdirectory(Cesium3DTilesSelection)
add_subdirectory(CesiumIonClient)
add_subdirectory(CesiumQuantizedMeshTerrain)

# Private Targets
if (CESIUM_TESTS_ENABLED)
    # enable_testing() MUST be called before add_subdirectory or no tests
    # will be found by ctest
    enable_testing()
    add_subdirectory(CesiumNativeTests)
endif()

add_subdirectory(doc)

# Installation of third-party libraries required to use cesium-native

install(DIRECTORY extern/glm/glm
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT GLM
)

install(TARGETS tinyxml2)

install(TARGETS Async++)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/extern/asyncplusplus/include/async++.h TYPE INCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/asyncplusplus/include/async++ TYPE INCLUDE)

install(TARGETS spdlog)
install(DIRECTORY ${CESIUM_NATIVE_SPDLOG_INCLUDE_DIR}/spdlog DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(TARGETS ${CESIUM_NATIVE_DRACO_LIBRARY})

install(TARGETS sqlite3)

install(TARGETS modp_b64)

install(TARGETS httplib)

# Don't install CSPRNG when building for Universal Windows Platform
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore")
  install(TARGETS csprng)
endif()

# ktx_read's PUBLIC_HEADER has paths relative to its own directory, so the install line below will fail.
# We could fix that, but we don't need the KTX public headers installed anyway (they should be considered
# private to cesium-native), so just set the PUBLIC_HEADER to an empty string.
set_target_properties(ktx_read PROPERTIES PUBLIC_HEADER "")
install(TARGETS ktx_read)

install(TARGETS webpdecoder)

install(DIRECTORY $<TARGET_LINKER_FILE:turbojpeg> TYPE LIB)

#install(DIRECTORY $<TARGET_LINKER_FILE:zlibstatic> TYPE LIB)

install(DIRECTORY ${CESIUM_NATIVE_RAPIDJSON_INCLUDE_DIR}/rapidjson TYPE INCLUDE)

install(TARGETS s2geometry)

install(TARGETS expected-lite)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/expected-lite/include/nonstd TYPE INCLUDE)

install(TARGETS meshoptimizer)

install(TARGETS ada)

endif()