# Configuration Options
option(WITH_GOOGLE_TEST "Enable GoogleTest unit testing" ${WITH_TESTS})
if(WITH_GOOGLE_TEST)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        # Latest stable release that supports C++20
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(googletest)

    # Don't install
    option(INSTALL_GTEST "Enable installation of googletest." OFF)

    file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*_test.c[pp]?")
    add_executable(${PROJECT_NAME}_test ${TEST_FILES})

    target_include_directories(${PROJECT_NAME}_test
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_test
        PRIVATE
        ${CMAKE_BINARY_DIR}/src
    )
    target_link_libraries(${PROJECT_NAME}_test PRIVATE GTest::gmock_main)
    add_link_options(${PROJECT_NAME}_test 
        PRIVATE
        "-Wl,--no-as-needed"
    )
    target_compile_definitions(${PROJECT_NAME}_test
        PRIVATE
        # Export some CMake options for convenience
        CMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
    )

    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}_test
        TEST_PREFIX "unit."
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        PROPERTIES "LABELS;gtest;unit"
    )

endif()