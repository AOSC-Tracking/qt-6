# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(cpp-qt-reserved-words LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(cpp-qt-reserved-words
    SOURCES
        tst_reservedwords.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Test
)

qt_add_library(ReservedWordsClient)
qt_add_openapi_client(ReservedWordsClient
    SPEC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/reserved-words.yaml
    ADDITIONAL_PROPERTIES
        cppNamespace=ReservedWords
    OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
)

qt_autogen_tools_initial_setup(ReservedWordsClient)
target_link_libraries(cpp-qt-reserved-words PRIVATE ReservedWordsClient)

if(WIN32)
    add_custom_command(TARGET cpp-qt-reserved-words POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:ReservedWordsClient>
            ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()
