# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Svg Module:
#####################################################################

# Handle the conditional finding of either system zlib or qt zlib.
if(NOT QT_FEATURE_system_zlib)
    find_package(Qt6 COMPONENTS ZlibPrivate)
elseif(NOT TARGET WrapZLIB::WrapZLIB)
    qt_find_package(WrapZLIB PROVIDED_TARGETS WrapZLIB::WrapZLIB)
endif()

qt_internal_add_module(Svg
    SOURCES
        qsvgfont.cpp qsvgfont_p.h
        qsvggenerator.cpp qsvggenerator.h
        qsvggraphics.cpp qsvggraphics_p.h
        qsvghandler.cpp qsvghandler_p.h
        qsvgnode.cpp qsvgnode_p.h
        qsvgrenderer.cpp qsvgrenderer.h
        qsvgstructure.cpp qsvgstructure_p.h
        qsvgfilter.cpp qsvgfilter_p.h
        qsvgstyle.cpp qsvgstyle_p.h
        qsvgtinydocument.cpp qsvgtinydocument_p.h
        qsvghelper_p.h
        qsvgdebug.cpp
        qsvgvisitor.cpp qsvgvisitor_p.h
        qtsvgglobal.h qtsvgglobal_p.h
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
        QT_NO_USING_NAMESPACE
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(Svg CONDITION MSVC AND (TEST_architecture_arch STREQUAL "i386")
    LINK_OPTIONS
        "/BASE:0x66000000"
)

qt_internal_extend_target(Svg CONDITION QT_FEATURE_system_zlib
    LIBRARIES
        WrapZLIB::WrapZLIB
)

qt_internal_extend_target(Svg CONDITION NOT QT_FEATURE_system_zlib
    LIBRARIES
        Qt::ZlibPrivate
)
qt_internal_add_docs(Svg
    doc/qtsvg.qdocconf
)
