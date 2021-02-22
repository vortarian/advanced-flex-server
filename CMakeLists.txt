cmake_minimum_required(VERSION 3.16)


#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=c2x ")

project(advanced-flex-server)

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

# @see: https://github.com/conan-io/cmake-conan
conan_cmake_run(
    CONANFILE conanfile.txt
    BASIC_SETUP
    BUILD missing
    # This means that the default profile was setup on the system and will be used
    #PROFILE default
    )

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup(TARGETS)

add_definitions(-D_REENTRANT -DBOOST_NO_DEPRECATED )
#add_definitions(-DBOOST_ALL_DYN_LINK)
add_definitions(-DBOOST_ALL_NO_LIB)

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if (CMAKE_CXX_CPPCHECK)
  if(NOT DEFINED CONAN_USER_HOME)
    set(CONAN_USER_HOME "${HOME}")
  endif()
  list(
    APPEND CMAKE_CXX_CPPCHECK
      "${COMPILE_DEFINITIONS}"
      "--enable=all"
      "--inconclusive"
      #"--force"
      #"--inline-suppr"
      "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppCheck.suppressions"
      #"--suppress=missingIncludeSystem"
  )
endif()

include_directories(src/c++)

set(STANDARD_LIBRARIES
    ${CONAN_LIBS}
#    c++ # c++ is needed for linking with clang-10 when using libc++ in the conan container on gitlab
    ${STANDARD_LIBRARIES}
  )
get_property(STANDARD_INCLUDES DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
message(STATUS "CONAN_INCLUDE_DIRS: ${CONAN_INCLUDE_DIRS}")
message(STATUS "CONAN_LIBS: ${CONAN_LIBS}")
message(STATUS "STANDARD_INCLUDES: ${STANDARD_INCLUDES}")
message(STATUS "STANDARD_LIBRARIES: ${STANDARD_LIBRARIES}")

add_custom_target(
        build-dependencies.html ALL
        COMMAND "conan info .. --graph=dependencies.html")

# STUB Target meant only to execise all the dependencies so they get built in the prepare/caching stage of cicd
add_custom_target(
        dependencies ALL
        # This target executes everything up to this step which includes conan install
        COMMAND echo "Dependencies installed"
)
add_executable(client
        tst/c++/systemicai/http/client/api.cpp
        src/c++/systemicai/common/certificate.h
        src/c++/systemicai/http/client/settings.h
        src/c++/systemicai/http/client/api.hpp)
target_link_libraries(client ${STANDARD_LIBRARIES})

add_executable(${PROJECT_NAME}
   src/c++/systemicai/cmd/httpd.cpp
   src/c++/systemicai/http/server/handler.hpp
   src/c++/systemicai/common/certificate.h
   src/c++/systemicai/http/server/settings.h
   src/c++/systemicai/http/server/server.hpp)
target_link_libraries(${PROJECT_NAME} ${STANDARD_LIBRARIES})

add_executable(unit-tests tst/c++/systemicai/unit_tests.cpp)
target_include_directories(unit-tests PRIVATE tst/c++)
target_link_libraries(unit-tests ${STANDARD_LIBRARIES} -fprofile-instr-generate -fcoverage-mapping)
target_compile_options(unit-tests PRIVATE -fprofile-instr-generate -fcoverage-mapping)

add_custom_target(
    coverage-reports
    DEPENDS unit-tests
    COMMAND LLVM_PROFILE_FILE=coverage/unit-tests.profraw bin/unit-tests --detect_memory_leaks --report_level=detailed --report_format=XML --report_sink=unit-tests.report.xml
)

add_custom_target(
    coverage ALL
    DEPENDS coverage-reports
    COMMAND llvm-profdata merge -sparse coverage/*.profraw -o coverage/coverage.profdata
    COMMAND llvm-cov report bin/unit-tests -instr-profile=coverage/coverage.profdata ${CMAKE_SOURCE_DIR}/src | tee -a coverage/coverage.txt
)

add_custom_target(
    cppcheck ALL
    COMMAND ${CMAKE_CXX_CPPCHECK} ${CMAKE_SOURCE_DIR}/src/c++ ${CMAKE_SOURCE_DIR}/tst/c++
)

add_custom_target(
    do_always ALL
    DEPENDS ${TARGETS} ${PROJECT_NAME} client test-server-settings test-server-service
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/dist
    COMMAND cp -rv bin ${CMAKE_SOURCE_DIR}/tst/html ${CMAKE_BINARY_DIR}/dist/
    # lib isn't always in projects
    COMMAND cp -rv lib ${CMAKE_BINARY_DIR}/dist/ || true
)

add_custom_target(
        conan_package
        ALL
        DEPENDS ${TARGETS} ${PROJECT_NAME} client do_always
        COMMAND echo \"If doing a local release:\"
        COMMAND echo \"To perform a release:\"
        COMMAND echo \"(example) conan create ${CMAKE_SOURCE_DIR}\"
        COMMAND echo \"(example) conan upload -r systemicai-afshttp advanced-flex-server/0.9 --all\"
)

