// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"

#include "client/qtapi.h"
#include "client/mixedapi.h"
#include "client/qobjectlike.h"
#include "client/qmetaenum.h"

#include <QtCore/qobject.h>
#include <QtCore/QMetaEnum>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace ReservedWords;

// If any reserved word is not escaped, the generated code will not compile:
// Qt keywords (emit, foreach, forever, signals, slots) would be expanded by the
// preprocessor and QObject methods (connect, disconnect) would clash with
// QObject's own methods. Compilation IS the primary verification.
// The runtime verifications below additionally confirm the exact identifier names.

class tst_ReservedWords : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void qtReservedWordsAsModelProperties();
    void cppKeywordsAsModelProperties();
    void qtReservedWordsAsOperationName();
    void reservedEnumValues();
};

void tst_ReservedWords::qtReservedWordsAsModelProperties()
{
    // Qt keywords: signals, slots, emit, foreach, forever
    QObjectLike obj;
    obj.setRSignals("test"_L1);
    QCOMPARE(obj.getRSignalsValue(), "test"_L1);
    obj.setRSlots("test"_L1);
    QCOMPARE(obj.getRSlotsValue(), "test"_L1);
    obj.setREmit(true);
    QCOMPARE(obj.isREmitValue(), true);
    obj.setRForeach("test"_L1);
    QCOMPARE(obj.getRForeachValue(), "test"_L1);
    obj.setRForever(false);
    QCOMPARE(obj.isRForeverValue(), false);

    // QObject methods: connect, disconnect
    obj.setRConnect("test"_L1);
    QCOMPARE(obj.getRConnectValue(), "test"_L1);
    obj.setRDisconnect("test"_L1);
    QCOMPARE(obj.getRDisconnectValue(), "test"_L1);
}

void tst_ReservedWords::cppKeywordsAsModelProperties()
{
    // class and struct: C++ keywords (handled by the upstream AbstractCppCodegen)
    QObjectLike obj;
    obj.setRClass("test"_L1);
    QCOMPARE(obj.getRClassValue(), "test"_L1);
    obj.setRStruct("test"_L1);
    QCOMPARE(obj.getRStructValue(), "test"_L1);
}

void tst_ReservedWords::qtReservedWordsAsOperationName()
{
    const char *warningMsg = "Access manager destroyed while 1 requests were still in progress";

    {
        ReservedWords::QtApi api;
        QTest::ignoreMessage(QtWarningMsg, warningMsg);
        api.r_signals();
    }

    {
        ReservedWords::QtApi api;
        QObjectLike body;
        body.setRSignals("test"_L1);
        body.setRSlots("test"_L1);
        QTest::ignoreMessage(QtWarningMsg, warningMsg);
        api.r_connect("test"_L1, body);
    }

    {
        ReservedWords::MixedApi mixedApi;
        QTest::ignoreMessage(QtWarningMsg, warningMsg);
        mixedApi.r_operator("test"_L1);
    }

    {
        ReservedWords::QtApi api;
        QTest::ignoreMessage(QtWarningMsg, warningMsg);
        api.r_username();
    }
}

void tst_ReservedWords::reservedEnumValues()
{
    // All enum values are uppercased. Qt/C++ reserved words that are reserved only
    // when lowercase (signals, class, int…) become safe after uppercasing, so they
    // do not need the reserved-word prefix.
    using E = ReservedWords::QMetaEnum::eQMetaEnum;
    QVERIFY(E::SIGNALS != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::SLOTS   != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::EMIT    != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::FOREACH != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::FOREVER != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::INT     != E::INVALID_VALUE_OPENAPI_GENERATED);
    QVERIFY(E::CLASS   != E::INVALID_VALUE_OPENAPI_GENERATED);
    // "null" is different: NULL is reserved when uppercase.
    // So it should be escaped with the reserved-word prefix instead.
    QVERIFY(E::r_NULL  != E::INVALID_VALUE_OPENAPI_GENERATED);
}

QTEST_MAIN(tst_ReservedWords)
#include "tst_reservedwords.moc"
