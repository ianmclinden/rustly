cmake_minimum_required(VERSION 3.20)

# Global configuration options
option(WITH_ALL "Enable all configuration options" OFF)
option(WITH_TESTS "Enable all source code testing" ${WITH_ALL})
# Installation options
option(WITH_INSTALL "Enable all installation options" ON)
option(WITH_HDR_LIBRARY "Enable installing library headers" ${WITH_INSTALL})
option(WITH_CMAKE_MODULE "Enable installing as a CMake module" ${WITH_INSTALL})

project(rustly
    VERSION 0.1.0
    DESCRIPTION "Rust-like functionalty for C++"
    HOMEPAGE_URL "https://github.com/ianmclinden/rustly/"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

include(GNUInstallDirs)
target_include_directories(${PROJECT_NAME} INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)

if(WITH_HDR_LIBRARY)
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/rustly DESTINATION include)
endif()

if(WITH_CMAKE_MODULE)
    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
    write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
    install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif()

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()