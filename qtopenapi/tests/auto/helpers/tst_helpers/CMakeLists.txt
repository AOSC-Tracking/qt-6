# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-helpers LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(cpp-qt-helpers
    SOURCES
        tst_helpers.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::OpenApiCommon
        Qt::Test
)

qt_add_library(HelpersTestClient)
qt_add_openapi_client(HelpersTestClient
    SPEC_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers-test.yaml
)

qt_autogen_tools_initial_setup(HelpersTestClient)
target_link_libraries(cpp-qt-helpers PRIVATE HelpersTestClient)

if(WIN32)
    add_custom_command(TARGET cpp-qt-helpers POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:HelpersTestClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
