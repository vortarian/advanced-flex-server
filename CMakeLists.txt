cmake_minimum_required(VERSION 3.10)
project(advanced-flex-server)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 17)
set(CXX clang++)
set(CC clang)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(BASIC_SETUP CONANFILE conanfile.txt BUILD missing)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

IF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
  message(STATUS "Windows Builds")
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/install-dependencies.bat")
  file (STRINGS ${CMAKE_SOURCE_DIR}/.deps DEPENDENCIES)
ELSE()
  message(STATUS "Linux Builds")
  execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/install-dependencies.sh)
  file (STRINGS ${CMAKE_SOURCE_DIR}/.deps DEPENDENCIES)
ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")

execute_process(COMMAND "conan info --graph=dependencies.html")

add_definitions(-D_REENTRANT -DBOOST_NO_DEPRECATED )
#add_definitions(-DBOOST_ALL_DYN_LINK)
add_definitions(-DBOOST_ALL_NO_LIB)

message(STATUS "DEPENDENCIES: ${DEPENDENCIES}")

include_directories(src/c++)
include_directories("${DEPENDENCIES}/date/include")
include_directories("${DEPENDENCIES}/uri/include")

set(STANDARD_LIBRARIES
    ${DEPENDENCIES}/uri/lib/libnetwork-uri.a
    ${CONAN_LIBS}
    c++ # c++ is needed for linking with clang-10 in the conan container on gitlab
    ${STANDARD_LIBRARIES}
  )
get_property(STANDARD_INCLUDES DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "CONAN_INCLUDE_DIRS: ${CONAN_INCLUDE_DIRS}")
message(STATUS "CONAN_LIBS: ${CONAN_LIBS}")
message(STATUS "STANDARD_INCLUDES: ${STANDARD_INCLUDES}")
message(STATUS "STANDARD_LIBRARIES: ${STANDARD_LIBRARIES}")

add_executable(client
        tst/c++/systemicai/http/client/api.cpp
        src/c++/systemicai/common/certificate.h
        src/c++/systemicai/http/client/settings.h
        src/c++/systemicai/http/client/api.hpp)
target_link_libraries(client ${STANDARD_LIBRARIES})

add_executable(${PROJECT_NAME}
   tst/c++/systemicai/http/server/server.cpp
   src/c++/systemicai/http/server/handler.hpp
   src/c++/systemicai/common/certificate.h
   src/c++/systemicai/http/server/settings.h
   src/c++/systemicai/http/server/server.hpp)
target_link_libraries(${PROJECT_NAME} ${STANDARD_LIBRARIES})

add_executable(test-server-settings
    tst/c++/systemicai/http/server/settings_test.cpp
    src/c++/systemicai/http/server/settings.h
  )

add_custom_target(
    unit_tests ALL
    DEPENDS test-server-settings
    COMMAND bin/test-server-settings --report_level=detailed
)

add_custom_target(
    do_always ALL
    DEPENDS ${TARGETS} ${PROJECT_NAME} client test-server-settings
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist
    COMMAND cp -rv bin ${CMAKE_SOURCE_DIR}/tst/html ${CMAKE_BINARY_DIR}/dist/
    # lib isn't always in projects
    COMMAND cp -rv lib ${CMAKE_BINARY_DIR}/dist/ || true
)

add_custom_target(
        conan_package
        ALL
        DEPENDS ${TARGETS} ${PROJECT_NAME} client do_always
        COMMAND echo \"To perform a release:\"
        COMMAND echo \"(example) conan create ${CMAKE_SOURCE_DIR}\"
        COMMAND echo \"(example) conan upload -r systemicai-afshttp advanced-flex-server/0.9 --all\"
)

