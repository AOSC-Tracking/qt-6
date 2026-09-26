// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0

package org.qtproject.qt.codegen;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import org.qtproject.qt.codegen.CppQt6ClientGenerator;

public class CppQt6ClientGeneratorTest {

    /** Verifies that reserved words are correctly escaped by toVarName(). */
    @Test
    public void testReservedWords() {
        CppQt6ClientGenerator codegen = new CppQt6ClientGenerator();

        String[] reservedWords = {
            "signals", "slots", "emit", "foreach", "forever", // Qt keywords (CppQt6AbstractCodegen)
            "connect", "disconnect",                          // QObject methods (CppQt6AbstractCodegen)
            "valid", "set",                                   // clash with generated isValid()/isSet() (CppQt6ClientGenerator)
            "class", "template", "operator", "delete", "new", // C++ keywords (AbstractCppCodegen)
            "namespace", "explicit", "inline", "virtual",     // C++ keywords (AbstractCppCodegen)
        };
        for (String word : reservedWords) {
            Assertions.assertEquals(codegen.escapeReservedWord(word), codegen.toVarName(word),
                                    "Expected '" + word + "' to be escaped by toVarName()");
        }

        // Sanity check: ordinary words must pass through toVarName() unchanged.
        Assertions.assertEquals("value", codegen.toVarName("value"));
        Assertions.assertEquals("name",  codegen.toVarName("name"));
    }

    /** Verifies that {{prefix}}BaseApi method names are correctly escaped by toOperationId(). */
    @Test
    public void testApiReservedWords() {
        CppQt6ClientGenerator codegen = new CppQt6ClientGenerator();

        String[] apiReservedWords = {
            "username", "password", "apiKey", "bearerToken", // {{prefix}}BaseApi methods (CppQt6ClientGenerator)
        };
        for (String word : apiReservedWords) {
            Assertions.assertEquals(codegen.escapeReservedWord(word), codegen.toOperationId(word),
                                    "Expected '" + word + "' to be escaped by toOperationId()");
        }

        // Sanity check: ordinary operation names must pass through unchanged.
        Assertions.assertEquals("getValue", codegen.toOperationId("getValue"));
    }
}
