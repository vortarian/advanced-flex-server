cmake_minimum_required(VERSION 3.10)
project(advanced-flex-server)

set(CMAKE_CXX_STANDARD 17)

IF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
  message(STATUS "Windows Builds")
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/install-dependencies.bat")
  file (STRINGS ${CMAKE_SOURCE_DIR}/.deps DEPENDENCIES)
  include("${DEPENDENCIES}/vcpkg-windows/scripts/buildsystems/vcpkg.cmake")
ELSE()
  message(STATUS "Linux Builds")
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/install-dependencies.sh)
  file (STRINGS ${CMAKE_SOURCE_DIR}/.deps DEPENDENCIES)
  include("${DEPENDENCIES}/vcpkg-linux/scripts/buildsystems/vcpkg.cmake")
ENDIF(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")


add_definitions(-DBOOST_ALL_DYN_LINK -D_REENTRANT -DBOOST_NO_DEPRECATED )

message(STATUS "DEPENDENCIES: ${DEPENDENCIES}")

set(EXECUTABLE_OUTPUT_PATH dist/bin)
set(LIBRARY_OUTPUT_PATH dist/lib)

# Setup our custom dependencies
#set(OPENSSL_ROOT_DIR "${DEPENDENCIES}/openssl")
find_package(OpenSSL REQUIRED)
#set(BOOST_ROOT "${DEPENDENCIES}/boost")
find_package(Boost 1.75.0 REQUIRED COMPONENTS random chrono regex log log_setup thread filesystem system )

find_package(Threads REQUIRED)

include_directories(src/c++)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Threads_INCLUDE_DIRS})
include_directories("${DEPENDENCIES}/date/include")
include_directories("${DEPENDENCIES}/uri/include")

set(STANDARD_LIBRARIES
  ${Boost_LIBRARIES}
  Threads::Threads
  ${OPENSSL_LIBRARIES}
  ${DEPENDENCIES}/uri/lib/libnetwork-uri.a
  )
get_property(STANDARD_INCLUDES DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
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
   src/c++/systemicai/common/certificate.h
   src/c++/systemicai/http/server/settings.h
   src/c++/systemicai/http/server/server.hpp)
target_link_libraries(server ${STANDARD_LIBRARIES})

install(PROGRAMS DESTINATION bin)

set(COPY_LIBRARIES ${STANDARD_LIBRARIES})
list(FILTER COPY_LIBRARIES INCLUDE REGEX "\\.so\\.*$" )
file(COPY ${COPY_LIBRARIES} DESTINATION dist/lib/)
file(COPY cfg DESTINATION dist/)
