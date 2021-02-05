cmake_minimum_required(VERSION 3.10)
project(advanced-flex-server)

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
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/install-dependencies.sh)
  file (STRINGS ${CMAKE_SOURCE_DIR}/.deps DEPENDENCIES)
ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")


add_definitions(-D_REENTRANT -DBOOST_NO_DEPRECATED )
#add_definitions(-DBOOST_ALL_DYN_LINK)
add_definitions(-DBOOST_ALL_NO_LIB)

message(STATUS "DEPENDENCIES: ${DEPENDENCIES}")

set(EXECUTABLE_OUTPUT_PATH dist/bin)
set(LIBRARY_OUTPUT_PATH dist/lib)

include_directories(src/c++)
include_directories("${DEPENDENCIES}/date/include")
include_directories("${DEPENDENCIES}/uri/include")

set(STANDARD_LIBRARIES
    ${DEPENDENCIES}/uri/lib/libnetwork-uri.a
    ${CONAN_LIBS}
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

add_executable(server
   tst/c++/systemicai/http/server/server.cpp
   src/c++/systemicai/http/server/handler.hpp
   src/c++/systemicai/common/certificate.h
   src/c++/systemicai/http/server/settings.h
   src/c++/systemicai/http/server/server.hpp)
target_link_libraries(server ${STANDARD_LIBRARIES})

install(PROGRAMS DESTINATION bin)

set(COPY_LIBRARIES ${STANDARD_LIBRARIES})
list(FILTER COPY_LIBRARIES INCLUDE REGEX "\\.so\\.*$" )
file(COPY ${COPY_LIBRARIES} DESTINATION dist/lib/)
file(COPY cfg DESTINATION dist/)
file(COPY tst/html DESTINATION dist/)
