if (NOT USE_CESIUM_OLD_BUILD)

# The bulk of the external dependencies are now automatically downlaoded and built using vcpkg.

if (WIN32)
  set(EXTERN_LIB_SUFFIX "lib")
  set(EXTERN_LIB_PREFIX "")
else()
  set(EXTERN_LIB_SUFFIX "a")
  set(EXTERN_LIB_PREFIX "lib")
endif()

add_library(include_helpers INTERFACE)
target_include_directories(include_helpers INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(earcut INTERFACE)
find_path(EARCUT_HPP_INCLUDE_DIRS "mapbox/earcut.hpp")
target_include_directories(earcut INTERFACE "${EARCUT_HPP_INCLUDE_DIRS}")



# Modp_b64 doesn't contain a CMake config, so we construct a fake import library here
add_library(modp_b64::modp_b64 STATIC IMPORTED GLOBAL)
set_target_properties(modp_b64::modp_b64 PROPERTIES
    INCLUDE_DIRECTORIES "${PACKAGE_BUILD_DIR}/include"
    # Yes, on non-windows platforms the name of the library really is liblibmodpbase64.a
    # Don't ask me, I didn't do it.
    IMPORTED_LOCATION "${PACKAGE_BUILD_DIR}/lib/${EXTERN_LIB_PREFIX}libmodpbase64.${EXTERN_LIB_SUFFIX}"
    IMPORTED_LOCATION_DEBUG "${PACKAGE_BUILD_DIR}/debug/lib/${EXTERN_LIB_PREFIX}libmodpbase64.${EXTERN_LIB_SUFFIX}"
)

# picosha2 also doesn't contain a CMake config, so we construct a fake header-only import library here
add_library(picosha2::picosha2 IMPORTED INTERFACE GLOBAL)
set_target_properties(picosha2::picosha2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_BUILD_DIR}/include"
)

find_library(ZLIB_NG_RELEASE_LIB NAMES zlibstatic-ng z-ng PATHS ${PACKAGE_BUILD_DIR}/lib NO_DEFAULT_PATH)
find_library(ZLIB_NG_DEBUG_LIB NAMES zlibstatic-ngd z-ng PATHS ${PACKAGE_BUILD_DIR}/debug/lib NO_DEFAULT_PATH)
add_library(zlib-ng::zlib-ng STATIC IMPORTED GLOBAL)
target_include_directories(zlib-ng::zlib-ng INTERFACE "${PACKAGE_BUILD_DIR}/include")
set_target_properties(zlib-ng::zlib-ng PROPERTIES
  IMPORTED_LOCATION "${ZLIB_NG_RELEASE_LIB}"
  IMPORTED_LOCATION_DEBUG "${ZLIB_NG_DEBUG_LIB}"
)

else() # USE_CESIUM_OLD_BUILD

if (MSVC)
else()
	add_compile_options(-Wno-error -Wno-error=unused-but-set-variable -Wno-sign-conversion -Wno-error=shorten-64-to-32 -Wno-error=sign-conversion)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/Catch2/contrib")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)

if (NOT TARGET Catch2 AND CESIUM_TESTS_ENABLED)
  add_subdirectory(Catch2)
endif()

option(KTX_FEATURE_STATIC_LIBRARY "" on)
option(KTX_FEATURE_TESTS "" off)

# cesium-native uses std::variant features that require iOS 12+. Don't let KTX force
# our build to use iOS 11, because it won't work.
if(IOS)
  set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0" CACHE STRING "iOS Deployment Target")
endif()

add_subdirectory(KTX-Software)

option(WEBP_BUILD_ANIM_UTILS "" off)
option(WEBP_BUILD_CWEBP "" off)
option(WEBP_BUILD_DWEBP "" off)
option(WEBP_BUILD_EXTRAS "" off)
option(WEBP_BUILD_GIF2WEBP "" off)
option(WEBP_BUILD_IMG2WEBP "" off)
option(WEBP_BUILD_LIBWEBPMUX "" off)
option(WEBP_BUILD_VWEBP "" off)
option(WEBP_BUILD_WEBPINFO "" off)
option(WEBP_BUILD_WEBPMUX "" off)
option(WEBP_NEAR_LOSSLESS "" off)
option(WEBP_UNICODE "" off)
add_subdirectory(libwebp)

if ((NOT TARGET draco) AND (NOT TARGET draco_static))
    add_subdirectory(draco)
    if (MSVC)
      set(CESIUM_NATIVE_DRACO_LIBRARY draco)
    else()
      set(CESIUM_NATIVE_DRACO_LIBRARY draco_static)
    endif()
endif()

set(CESIUM_NATIVE_DRACO_LIBRARY ${CESIUM_NATIVE_DRACO_LIBRARY} PARENT_SCOPE)

add_subdirectory(meshoptimizer)

if (NOT TARGET glm)
    add_subdirectory(glm GLM)
endif()

# tinyxml2's CMake build tries to generate a pkg-config file in a way that's incompatible with multi-config generators
# (e.g., Visual Studio). And anyway it's trivial to build manually, so we do that here.
add_library(tinyxml2 tinyxml2/tinyxml2.cpp)
target_include_directories(
    tinyxml2
    PUBLIC
        tinyxml2
)

add_subdirectory(asyncplusplus)

add_subdirectory(fmt)

set(SPDLOG_BUILD_TESTING OFF CACHE INTERNAL "Disable SPDLOG Testing")
set(SPDLOG_FMT_EXTERNAL ON CACHE INTERNAL "Enable External FMT")
add_subdirectory(spdlog)
target_compile_definitions(spdlog INTERFACE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)

if (NOT TARGET sqlite3)
  add_subdirectory(sqlite3)
endif()

add_subdirectory(modp_b64)

add_subdirectory(expected-lite)

set(ABSL_PROPAGATE_CXX_STD ON)
add_definitions(-DABSL_MIN_LOG_LEVEL=1)
add_subdirectory(abseil)

# s2geometry's CMake requires OpenSSL, even though it's not needed for any of
# the functionality we actually use. So a simple library with enough functionality
# for our needs is defined here.
add_library(s2geometry
  s2geometry/src/s2/s2cell_id.cc
  s2geometry/src/s2/r2rect.cc
  s2geometry/src/s2/s2coords.cc 
  s2geometry/src/s2/util/coding/coder.cc
  s2geometry/src/s2/s2latlng.cc    
  s2geometry/src/s2/s1angle.cc
  s2geometry/src/s2/base/malloc_extension.cc  
  s2geometry/src/s2/s1interval.cc  
)

target_include_directories(s2geometry SYSTEM PUBLIC 
    s2geometry/src 
    abseil
)

target_link_libraries(s2geometry PUBLIC    
    absl::base
    absl::btree
    absl::check
    absl::config
    absl::core_headers
    absl::dynamic_annotations
    absl::endian
    absl::fixed_array
    absl::flags
    absl::flat_hash_map
    absl::flat_hash_set
    absl::hash
    absl::inlined_vector
    absl::int128
    absl::log
    absl::log_severity
    absl::memory
    absl::span
    absl::status
    absl::str_format
    absl::strings
    absl::type_traits
    absl::utility
    absl::vlog_is_on  
)

if (MSVC)
  target_compile_options(s2geometry PRIVATE "/J" "/wd4127" "/wd4996")
  target_compile_definitions(s2geometry PRIVATE NOMINMAX _USE_MATH_DEFINES)
endif()

set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "Don't use Brotli")
#set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "Don't use Zlib")
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "Don't use OpenSSL")
add_subdirectory(cpp-httplib)

# CSPRNG's CMake setup is old school, and it really only has one source
# file, so set up a library manually here.
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "WindowsStore")
  add_library(csprng CSPRNG/source/csprng.cpp)
  target_include_directories(
      csprng
      SYSTEM PUBLIC
          CSPRNG/source
  )
  set(CSPRNG_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/CSPRNG/source/duthomhas/csprng.h ${CMAKE_CURRENT_SOURCE_DIR}/CSPRNG/source/duthomhas/csprng.hpp ${CMAKE_CURRENT_SOURCE_DIR}/CSPRNG/source/duthomhas/is_iterable.hpp)
endif()

# PicoSHA2 doesn't have CMakeLists.txt at all
add_library(PicoSHA2 INTERFACE)
target_include_directories(PicoSHA2 SYSTEM INTERFACE PicoSHA2)
set_target_properties(PicoSHA2 PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/PicoSHA2/picosha2.h")

# Earcut has an old school CMake setup as well with lots of global settings, and it's header only anyway.
# So define a simple library for it here.
add_library(earcut INTERFACE)
target_include_directories(earcut SYSTEM INTERFACE earcut/include)
set_target_properties(earcut PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/earcut/include/mapbox/earcut.hpp")

set(CESIUM_NATIVE_SPDLOG_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/spdlog/include" CACHE INTERNAL
    "Include directory for spdlog"
)

set(CESIUM_NATIVE_RAPIDJSON_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/rapidjson/include" CACHE INTERNAL
    "Include directory for rapidjson"
)

set(CESIUM_NATIVE_RAPIDJSON_DEFINES
    RAPIDJSON_HAS_CXX11_RVALUE_REFS
    CACHE INTERNAL
    "Compiler definitions for rapidjson"
)

set(CESIUM_NATIVE_DRACO_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/draco/src/" CACHE INTERNAL
    "Include directory for Draco"
)

set(CESIUM_NATIVE_STB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/stb" CACHE INTERNAL
    "Include directory for STB libraries"
)

set(CESIUM_NATIVE_LIBMORTON_INCUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/libmorton/include" CACHE INTERNAL
    "Include directory for libmorton libraries"
)

set(CESIUM_NATIVE_FMT_INCUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/fmt/include" CACHE INTERNAL
    "Include directory for fmt"
)

set(CESIUM_NATIVE_SQLITE3_INCUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/sqlite3" CACHE INTERNAL
    "Include directory for sqlite3"
)

set(BUILD_TESTING OFF CACHE INTERNAL "Disable libmorton Testing")
add_subdirectory(libmorton)

# The zlib cmake build makes its working directory dirty.
# So we make a copy of it to avoid that annoyance.
# file(COPY "${CMAKE_CURRENT_LIST_DIR}/zlib/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/zlib-src")
# add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/zlib-src ${CMAKE_CURRENT_BINARY_DIR}/zlib)

# The new CMakeLists-txt uses zlib-ng instead of zlib it seems
set(ZLIB_COMPAT, OFF)
set(ZLIB_ENABLE_TESTS, OFF)
set(WITH_GZFILEOP, ON)
set(WITH_OPTIM, ON)
set(WITH_NEW_STRATEGIES, ON)
set(WITH_NATIVE_INSTRUCTIONS, OFF)
set(WITH_RUNTIME_CPU_DETECTION, OFF)
set(WITH_SANITIZER, OFF)
set(WITH_GTEST, OFF)
set(WITH_FUZZERS, OFF)
set(WITH_BENCHMARKS, OFF)
set(WITH_MAINTAINER_WARNINGS, OFF)
set(WITH_CODE_COVERAGE, OFF)
add_subdirectory(zlib-ng)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    if(NOT IS_ABSOLUTE ${CMAKE_TOOLCHAIN_FILE})
        set(TJ_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/${CMAKE_TOOLCHAIN_FILE})
    else()
        set(TJ_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE})
    endif()
endif()

if(IOS AND XCODE AND NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(TJ_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
else()
    set(TJ_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo)
endif()

include(ExternalProject)
ExternalProject_Add(libjpeg-turbo
    SOURCE_DIR        "${CMAKE_CURRENT_LIST_DIR}/libjpeg-turbo"
    PREFIX            "libjpeg-turbo"
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -B ${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo -S ${CMAKE_CURRENT_LIST_DIR}/libjpeg-turbo -DCMAKE_INSTALL_PREFIX=${TJ_INSTALL_PREFIX} -DENABLE_SHARED=0 -DWITH_CRT_DLL=1 -G ${CMAKE_GENERATOR} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME} "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" -DCMAKE_SYSTEM_PROCESSOR=$<IF:$<BOOL:${CMAKE_SYSTEM_PROCESSOR}>,${CMAKE_SYSTEM_PROCESSOR},unknown> -DCMAKE_BUILD_TYPE=$<CONFIG> -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE} -DCMAKE_ANDROID_ARCH_ABI=${CMAKE_ANDROID_ARCH_ABI} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES} -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET} -DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX} -DCMAKE_TOOLCHAIN_FILE=${TJ_TOOLCHAIN_FILE} -DCMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE} -DANDROID_ABI=${CMAKE_ANDROID_ARCH_ABI}
    BUILD_COMMAND     ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libjpeg-turbo --config $<CONFIG> --target install
    INSTALL_COMMAND   ""
    BUILD_BYPRODUCTS ${TJ_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}turbojpeg${CMAKE_STATIC_LIBRARY_SUFFIX}
                     ${TJ_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}turbojpeg${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(turbojpeg STATIC IMPORTED GLOBAL)
set_property(TARGET turbojpeg PROPERTY IMPORTED_LOCATION ${TJ_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}turbojpeg${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET turbojpeg PROPERTY IMPORTED_LOCATION_DEBUG ${TJ_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}turbojpeg${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX})
target_include_directories(turbojpeg INTERFACE "${CMAKE_CURRENT_LIST_DIR}/libjpeg-turbo")
add_dependencies(turbojpeg libjpeg-turbo)

add_subdirectory(ada)

endif()