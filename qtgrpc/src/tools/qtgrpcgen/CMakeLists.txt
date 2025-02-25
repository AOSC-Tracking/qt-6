# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_tool_target_name(target_name qtgrpcgen)
qt_internal_add_tool(${target_name}
    TARGET_DESCRIPTION "Qt gRPC generator"
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    TOOLS_TARGET Grpc
    CORE_LIBRARY None
    EXCEPTIONS
    SOURCES
        main.cpp
        qgrpcgenerator.cpp qgrpcgenerator.h
        grpctemplates.cpp grpctemplates.h
        serverdeclarationprinter.cpp serverdeclarationprinter.h
        clientdeclarationprinter.cpp clientdeclarationprinter.h
        clientdefinitionprinter.cpp clientdefinitionprinter.h
        qmlclientdeclarationprinter.cpp qmlclientdeclarationprinter.h
        qmlclientdefinitionprinter.cpp qmlclientdefinitionprinter.h
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/${QT_CMAKE_EXPORT_NAMESPACE}GrpcToolsMacros.cmake"
    EXTRA_CMAKE_INCLUDES
        "${QT_CMAKE_EXPORT_NAMESPACE}GrpcToolsMacros.cmake"
    LIBRARIES
        WrapProtobuf::WrapLibProtoc
        QtProtocCommon
    ATTRIBUTION_FILE_DIR_PATHS
        ../../../
    ATTRIBUTION_ENTRY_INDEX 1
)

qt_internal_return_unless_building_tools()

qt_record_extra_package_dependency(${target_name} WrapProtoc "")

if(MSVC)
    # Disable C4251:
    # `class 'type1' needs to have dll-interface to be used by clients of class 'type2'`
    # which can be generated from some of the protobuf headers on msvc.
    # This warning is also disabled globally in Qt in QtCore/qcompilerdetection.h
    target_compile_options(${target_name}
        PRIVATE "/wd4251"
    )
    target_compile_definitions(${target_name}
        PRIVATE "_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING"
    )
endif()
