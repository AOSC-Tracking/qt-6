#!/bin/bash
# Copyright (C) 2026 The Qt Company Ltd
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

$HOME/bauhaus-suite/setup.sh --non-interactive
export PATH=/home/qt/bauhaus-suite/bin:$PATH
export BAUHAUS_CONFIG=$(cd $(dirname $(readlink -f $0)) && pwd)
export AXIVION_VERSION_NAME=$(git rev-parse HEAD)
export EXCLUDE_FILES="../qtcanvaspainter_build/*:src/3rdparty/*"
export MODULE=$TESTED_MODULE_COIN
export PACKAGE="Add-ons"
export IRNAME="../qtcanvaspainter_build/$TESTED_MODULE_COIN.ir"

ROOT_DIR=src
MAGIC="Qt-Security score:critical"

INCLUDE_FILES=""

while IFS= read -r file; do
    if [ -z "$INCLUDE_FILES" ]; then
        INCLUDE_FILES="$file"
    else
        INCLUDE_FILES="$INCLUDE_FILES:$file"
    fi
done <<EOF
$(grep -rl "$MAGIC" "$ROOT_DIR")
EOF

export INCLUDE_FILES

axivion_ci "$@"
