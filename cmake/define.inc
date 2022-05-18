CMAKE_MINIMUM_REQUIRED(VERSION 3.0...3.20)
PROJECT(TDengine)

IF (TD_ACCOUNT)
  ADD_DEFINITIONS(-D_ACCT)
ENDIF ()

IF (TD_ADMIN)
  ADD_DEFINITIONS(-D_ADMIN)
ENDIF ()

IF (TD_GRANT)
  ADD_DEFINITIONS(-D_GRANT)
ENDIF ()

IF (TD_USB_DONGLE)
  ADD_DEFINITIONS(-D_USB_DONGLE)
ENDIF ()

IF (TD_TSDB_PLUGINS)
  ADD_DEFINITIONS(-D_TSDB_PLUGINS)
ENDIF ()

IF (TD_STORAGE)
  ADD_DEFINITIONS(-D_STORAGE)
ENDIF ()

IF (TD_TOPIC)
  ADD_DEFINITIONS(-D_TOPIC)
ENDIF ()

IF (TD_MODULE)
  ADD_DEFINITIONS(-D_MODULE)
ENDIF ()

IF (TD_GODLL)
  ADD_DEFINITIONS(-D_TD_GO_DLL_)
ENDIF ()

IF (TD_MEM_CHECK)
  ADD_DEFINITIONS(-DTAOS_MEM_CHECK)
ENDIF ()

IF (TD_RANDOM_FILE_FAIL)
  ADD_DEFINITIONS(-DTAOS_RANDOM_FILE_FAIL)
ENDIF ()

IF (TD_RANDOM_NETWORK_FAIL)
  ADD_DEFINITIONS(-DTAOS_RANDOM_NETWORK_FAIL)
ENDIF ()

IF (TD_LINUX_64)
  ADD_DEFINITIONS(-D_M_X64)
  ADD_DEFINITIONS(-D_TD_LINUX_64)
  MESSAGE(STATUS "linux64 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -gdwarf-2 -msse4.2 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
  ADD_DEFINITIONS(-DUSE_LIBICONV)

  IF (JEMALLOC_ENABLED)
    ADD_DEFINITIONS(-DTD_JEMALLOC_ENABLED -I${CMAKE_BINARY_DIR}/build/include -L${CMAKE_BINARY_DIR}/build/lib -Wl,-rpath,${CMAKE_BINARY_DIR}/build/lib -ljemalloc)
  ENDIF ()

ENDIF ()

IF (TD_LINUX_32)
  ADD_DEFINITIONS(-D_TD_LINUX_32)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "linux32 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -fsigned-char -munaligned-access -fpack-struct=8 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
ENDIF ()

IF (TD_ARM_64)
  ADD_DEFINITIONS(-D_TD_ARM_64)
  ADD_DEFINITIONS(-D_TD_ARM_)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "arm64 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -fsigned-char -fpack-struct=8 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")

  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lua/src)
ENDIF ()

IF (TD_ARM_32)
  ADD_DEFINITIONS(-D_TD_ARM_32)
  ADD_DEFINITIONS(-D_TD_ARM_)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "arm32 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -fsigned-char -fpack-struct=8 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-incompatible-pointer-types ")

  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lua/src)
ENDIF ()

IF (TD_MIPS_64)
  ADD_DEFINITIONS(-D_TD_MIPS_)
  ADD_DEFINITIONS(-D_TD_MIPS_64)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "mips64 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -fsigned-char -fpack-struct=8 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
ENDIF ()

IF (TD_MIPS_32)
  ADD_DEFINITIONS(-D_TD_MIPS_)
  ADD_DEFINITIONS(-D_TD_MIPS_32)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "mips32 is defined")
  SET(COMMON_FLAGS "-Wall -Werror -fPIC -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
ENDIF ()

IF (TD_ALPINE)
  SET(COMMON_FLAGS "${COMMON_FLAGS} -Wl,-z,stack-size=2097152")
  link_libraries(argp)
  ADD_DEFINITIONS(-D_ALPINE)
  MESSAGE(STATUS "alpine is defined")
ENDIF ()

IF ("${BUILD_HTTP}" STREQUAL "")
  IF (TD_LINUX)
    IF (TD_ARM_32)
      SET(TD_BUILD_HTTP TRUE)
    ELSE ()
      SET(TD_BUILD_HTTP TRUE)
    ENDIF ()
  ELSEIF (TD_DARWIN)
    SET(TD_BUILD_HTTP TRUE)
  ELSE ()
    SET(TD_BUILD_HTTP TRUE)
  ENDIF ()
ELSEIF (${BUILD_HTTP} MATCHES "false")
  SET(TD_BUILD_HTTP FALSE)
ELSEIF (${BUILD_HTTP} MATCHES "true")
  SET(TD_BUILD_HTTP TRUE)
ELSEIF (${BUILD_HTTP} MATCHES "internal")
  SET(TD_BUILD_HTTP FALSE)
  SET(TD_BUILD_TAOSA_INTERNAL TRUE)
ELSE ()
  SET(TD_BUILD_HTTP TRUE)
ENDIF ()

IF (TD_BUILD_HTTP)
  ADD_DEFINITIONS(-DHTTP_EMBEDDED)
ENDIF ()

IF ("${BUILD_TOOLS}" STREQUAL "")
  IF (TD_LINUX)
    IF (TD_ARM_32)
      SET(BUILD_TOOLS "false")
    ELSEIF (TD_ARM_64)
      SET(BUILD_TOOLS "false")
    ELSE ()
      SET(BUILD_TOOLS "false")
    ENDIF ()
  ELSEIF (TD_DARWIN)
    SET(BUILD_TOOLS "false")
  ELSE ()
    SET(BUILD_TOOLS "false")
  ENDIF ()
ENDIF ()

IF ("${BUILD_TOOLS}" MATCHES "false")
    MESSAGE("${Yellow} Will _not_ build taos_tools! ${ColourReset}")
    SET(TD_TAOS_TOOLS FALSE)
ELSE ()
    MESSAGE("")
    MESSAGE("${Green} Will build taos_tools! ${ColourReset}")
    MESSAGE("")
    SET(TD_TAOS_TOOLS TRUE)
ENDIF ()

IF (${BUILD_LUA} MATCHES "false")
  SET(TD_BUILD_LUA FALSE)
ENDIF ()

IF (TD_BUILD_LUA)
  MESSAGE("Enable lua")
  ADD_DEFINITIONS(-DLUA_EMBEDDED)
  SET(LINK_LUA "lua")
ELSE ()
  MESSAGE("Disable lua")
  SET(LINK_LUA "")
ENDIF ()

IF (TD_LINUX)
  ADD_DEFINITIONS(-DLINUX)
  ADD_DEFINITIONS(-D_LINUX)
  ADD_DEFINITIONS(-D_TD_LINUX)
  ADD_DEFINITIONS(-D_REENTRANT -D__USE_POSIX -D_LIBC_REENTRANT)

  IF (TD_NINGSI_60)
    ADD_DEFINITIONS(-D_TD_NINGSI_60)
    MESSAGE(STATUS "set ningsi macro to true")
  ENDIF ()

  IF (TD_MEMORY_SANITIZER)
    IF (TD_ARCHLINUX)
      SET(DEBUG_FLAGS "-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment -O0 -g3 -DDEBUG")
    ELSE ()
      SET(DEBUG_FLAGS "-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment -static-libasan -O0 -g3 -DDEBUG")
    ENDIF ()
    MESSAGE(STATUS "${BoldRed}Will compile with memory sanitizer! ${ColourReset}")
  ELSE ()
    SET(DEBUG_FLAGS "-O0 -g3 -DDEBUG")
  ENDIF ()
  SET(RELEASE_FLAGS "-O3 -Wno-error")

  IF (${COVER} MATCHES "true")
    MESSAGE(STATUS "Test coverage mode, add extra flags")
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov --coverage")
    SET(COMMON_FLAGS "${COMMON_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
  ENDIF ()

  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/cJson/inc)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lz4/inc)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lua/src)
ENDIF ()

IF (TD_DARWIN_64)
  ADD_DEFINITIONS(-D_TD_DARWIN_64)
  ADD_DEFINITIONS(-DDARWIN)
  ADD_DEFINITIONS(-D_REENTRANT -D__USE_POSIX -D_LIBC_REENTRANT)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "darwin64 is defined")
  IF ("${CPUTYPE}" STREQUAL "apple_m1")
    SET(COMMON_FLAGS "-Wall -Werror -Wno-missing-braces -fPIC -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
  ELSE ()
    SET(COMMON_FLAGS "-Wall -Werror -Wno-missing-braces -fPIC -msse4.2 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
  ENDIF ()
  IF (TD_MEMORY_SANITIZER)
    SET(DEBUG_FLAGS "-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment -O0 -g3 -DDEBUG")
  ELSE ()
    SET(DEBUG_FLAGS "-O0 -g3 -DDEBUG")
  ENDIF ()
  SET(RELEASE_FLAGS "-Og")
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/cJson/inc)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lz4/inc)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lua/src)
ENDIF ()

IF (TD_WINDOWS)
  ADD_DEFINITIONS(-DWINDOWS)
  ADD_DEFINITIONS(-D__CLEANUP_C)
  ADD_DEFINITIONS(-DPTW32_STATIC_LIB)
  ADD_DEFINITIONS(-DPTW32_BUILD)
  ADD_DEFINITIONS(-D_MBCS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  SET(CMAKE_GENERATOR "NMake Makefiles" CACHE INTERNAL "" FORCE)
  IF (NOT TD_GODLL)
    SET(COMMON_FLAGS "/nologo /WX /wd4018 /wd4999 /Oi /Oy- /Gm- /EHsc /MT /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Gd /errorReport:prompt /analyze-")
    IF (MSVC AND (MSVC_VERSION GREATER_EQUAL 1900))
      SET(COMMON_FLAGS "${COMMON_FLAGS} /Wv:18")
    ENDIF ()

    IF (TD_MEMORY_SANITIZER)
      MESSAGE("memory sanitizer detected as true")
      SET(DEBUG_FLAGS "/fsanitize=address /Zi /W3 /GL")
    ELSE ()
      MESSAGE("memory sanitizer detected as false")
      SET(DEBUG_FLAGS "/Zi /W3 /GL")
    ENDIF ()
    SET(RELEASE_FLAGS "/W0 /O2 /GL")  # MSVC only support O2
  ENDIF ()

  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/pthread)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/iconv)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/regex)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/wepoll/inc)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/MsvcLibX/include)
  INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/deps/lua/src)
ENDIF ()

IF (TD_WINDOWS_64)
  ADD_DEFINITIONS(-D_M_X64)
  ADD_DEFINITIONS(-D_TD_WINDOWS_64)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "windows64 is defined")
ENDIF ()

IF (TD_WINDOWS_32)
  ADD_DEFINITIONS(-D_TD_WINDOWS_32)
  ADD_DEFINITIONS(-DUSE_LIBICONV)
  MESSAGE(STATUS "windows32 is defined")
ENDIF ()

IF (TD_LINUX)
  SET(COMMON_FLAGS "${COMMON_FLAGS} -pipe -Wshadow")
ENDIF ()

MESSAGE(STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID})
IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  EXECUTE_PROCESS( COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE clang_full_version_string )
  string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING ${clang_full_version_string})
  IF (CLANG_VERSION_STRING GREATER_EQUAL 13.1)
    SET(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-but-set-variable")
  ENDIF ()
ENDIF ()

INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/src/inc)
INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/src/os/inc)
INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/src/util/inc)
INCLUDE_DIRECTORIES(${TD_COMMUNITY_DIR}/src/common/inc)
