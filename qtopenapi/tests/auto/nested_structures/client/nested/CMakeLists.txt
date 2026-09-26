# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-nested LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if(NOT TARGET nested-server-go)
    message(WARNING "No nested-server-go server in the build.")
    return()
endif()

qt_internal_add_test(cpp-qt-nested
    SOURCES
        tst_nested.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(NestedData)
qt_add_openapi_client(NestedData
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/nested-structures.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=TestData
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_definitions(cpp-qt-nested
    PRIVATE
        SERVER_PATH="$<TARGET_FILE:nested-server-go>"
)
qt_autogen_tools_initial_setup(NestedData)
target_link_libraries(cpp-qt-nested PRIVATE NestedData)
add_dependencies(cpp-qt-nested nested-server-go)

if(WIN32)
    add_custom_command(TARGET cpp-qt-nested POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:NestedData>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
