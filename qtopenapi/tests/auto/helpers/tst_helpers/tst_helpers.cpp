// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/testenum.h"

#include <QtOpenApiCommon/qoaihelpers.h>

#include <QtCore/qobject.h>
#include <QtCore/qscopeguard.h>
#include <QtCore/qtemporaryfile.h>
#include <QtTest/qtest.h>

#include <limits>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

namespace QtOpenAPI {

class tst_Helpers : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void fromByteArray_QString_data();
    void fromByteArray_QString();
    void fromByteArray_QByteArray_data();
    void fromByteArray_QByteArray();
    void fromByteArray_QDateTime_data();
    void fromByteArray_QDateTime();
    void fromByteArray_QDate_data();
    void fromByteArray_QDate();
    void fromByteArray_bool_data();
    void fromByteArray_bool();
    void fromByteArray_qint32_data();
    void fromByteArray_qint32();
    void fromByteArray_qint64_data();
    void fromByteArray_qint64();
    void fromByteArray_float_data();
    void fromByteArray_float();
    void fromByteArray_double_data();
    void fromByteArray_double();
    void fromByteArray_enum_data();
    void fromByteArray_enum();
    void fromByteArray_object_data();
    void fromByteArray_object();
    void fromByteArray_httpFileElement();
    void fromJsonValue_QDateTime_data();
    void fromJsonValue_QDateTime();
    void fromJsonValue_QDateTime_nonStringInput();
    void fromJsonValue_QDate_data();
    void fromJsonValue_QDate();
    void fromJsonValue_QDate_nonStringInput();
};

void tst_Helpers::fromByteArray_QString_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QString>("expectedResult");

    QTest::newRow("non-empty string") << "Non empty string"_ba << QString("Non empty string"_L1);
    QTest::newRow("empty") << ""_ba << QString();
    QTest::newRow("null") << QByteArray() << QString();
    QTest::newRow("non-ASCII char") << "caf\xc3\xa9"_ba << u"café"_s;
    QTest::newRow("unicode") << "\xe2\x98\x83\xe2\x9d\xa4\xe4\xb8\x96\xe7\x95\x8c"_ba << u"☃❤世界"_s;
    QTest::newRow("very long string") << QByteArray(100000, 'x') << QString(100000, u'x');

    QByteArray withNull = "ab";
    withNull.append('\0');
    withNull.append("cd");
    QTest::newRow("embedded null") << withNull << QString::fromUtf8(withNull);
}

void tst_Helpers::fromByteArray_QString()
{
    QFETCH(QByteArray, input);
    QFETCH(QString, expectedResult);

    QString result;
    const bool ok = fromByteArray(input, result);
    QVERIFY(ok);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_QByteArray_data()
{
    QTest::addColumn<QByteArray>("input");

    QTest::newRow("non-empty") << "hello"_ba;
    QTest::newRow("empty") << ""_ba;
    QTest::newRow("null") << QByteArray();
    QTest::newRow("non-ASCII char") << "caf\xc3\xa9"_ba;

    QByteArray withNull = "ab";
    withNull.append('\0');
    withNull.append("cd");
    QTest::newRow("embedded null") << withNull;
}

void tst_Helpers::fromByteArray_QByteArray()
{
    QFETCH(QByteArray, input);

    QByteArray result;
    const bool ok = fromByteArray(input, result);
    QVERIFY(ok);
    QCOMPARE(result, input);
}

void tst_Helpers::fromByteArray_QDateTime_data()
{
    QTest::addColumn<QString>("format"); // empty = Qt::ISODate, "TextDate" = Qt::TextDate
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<QDateTime>("initialResult"); // initial value of result before the call
    QTest::addColumn<QDateTime>("expectedResult");

    const QDateTime expected(QDate(2026, 4, 27), QTime(12, 0, 0));

    QTest::newRow("ISO 8601 format - dd/MM/yyyy rejected")
            << QString() << "27/04/2026 12:00:00"_ba << false << QDateTime() << QDateTime();
    QTest::newRow("ISO 8601 format - valid")
            << QString() << "2026-04-27T12:00:00"_ba << true << QDateTime() << expected;
    QTest::newRow("ISO 8601 format - invalid string") // value left intact
            << QString() << "2000-not-a-datetime"_ba << false << expected << expected;
    QTest::newRow("custom format - valid")
            << u"dd/MM/yyyy HH:mm:ss"_s << "27/04/2026 12:00:00"_ba << true << QDateTime()
            << expected;
    QTest::newRow("custom format - ISO 8601 input rejected")
            << u"dd/MM/yyyy HH:mm:ss"_s << "2026-04-27T12:00:00"_ba << false << expected
            << expected; // value left intact
    QTest::newRow("TextDate format - valid")
            << u"TextDate"_s << "Mon Apr 27 12:00:00 2026"_ba << true << QDateTime() << expected;
}

void tst_Helpers::fromByteArray_QDateTime()
{
    QFETCH(QString, format);
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(QDateTime, initialResult);
    QFETCH(QDateTime, expectedResult);

    if (format.isEmpty())
        QVERIFY(setDateTimeFormat(Qt::ISODate));
    else if (format == "TextDate"_L1)
        QVERIFY(setDateTimeFormat(Qt::TextDate));
    else
        QVERIFY(setDateTimeFormat(format));
    const auto restoreFormat = qScopeGuard([] { setDateTimeFormat(Qt::ISODate); });

    QDateTime result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE_EQ(result, expectedResult);
}

void tst_Helpers::fromByteArray_QDate_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<QDate>("initialResult"); // initial value of result before the call
    QTest::addColumn<QDate>("expectedResult");

    const QDate expected(2026, 4, 27);

    QTest::newRow("dd/MM/yyyy format rejected")
            << "27/04/2026"_ba << false << QDate() << QDate();
    QTest::newRow("ISO 8601 format - valid")
            << "2026-04-27"_ba << true << QDate() << expected;
    QTest::newRow("invalid string") // value left intact
            << "2000-not-a-date"_ba << false << expected << expected;
}

void tst_Helpers::fromByteArray_QDate()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(QDate, initialResult);
    QFETCH(QDate, expectedResult);

    QDate result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE_EQ(result, expectedResult);
}

void tst_Helpers::fromByteArray_bool_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<bool>("initialResult"); // initial value of result before the call
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("true") << "true"_ba << true << false << true;
    QTest::newRow("false") << "false"_ba << true << true << false;
    QTest::newRow("truthy but not accepted") // value left intact
            << "1"_ba << false << false << false;
    QTest::newRow("empty") // value left intact
            << ""_ba << false << true << true;
}

void tst_Helpers::fromByteArray_bool()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(bool, initialResult);
    QFETCH(bool, expectedResult);

    bool result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_qint32_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<qint32>("initialResult"); // initial value of result before the call
    QTest::addColumn<qint32>("expectedResult");

    const qint64 overMax = static_cast<qint64>(std::numeric_limits<qint32>::max()) + 1;
    const qint64 underMin = static_cast<qint64>(std::numeric_limits<qint32>::min()) - 1;

    QTest::newRow("positive") << "42"_ba << true << qint32(0) << qint32(42);
    QTest::newRow("negative") << "-1"_ba << true << qint32(0) << qint32(-1);
    QTest::newRow("leading zeros") << "007"_ba << true << qint32(0) << qint32(7);
    QTest::newRow("max") << QByteArray::number(std::numeric_limits<qint32>::max())
                         << true << qint32(0) << std::numeric_limits<qint32>::max();
    QTest::newRow("min") << QByteArray::number(std::numeric_limits<qint32>::min())
                         << true << qint32(0) << std::numeric_limits<qint32>::min();
    QTest::newRow("overflow") // value left intact
            << QByteArray::number(overMax) << false << qint32(99) << qint32(99);
    QTest::newRow("underflow") // value left intact
            << QByteArray::number(underMin) << false << qint32(99) << qint32(99);
    QTest::newRow("not a number") // value left intact
            << "not_a_number"_ba << false << qint32(99) << qint32(99);
    QTest::newRow("empty") // value left intact
            << ""_ba << false << qint32(99) << qint32(99);
    QTest::newRow("decimal point") // value left intact
            << "42.5"_ba << false << qint32(99) << qint32(99);
    QTest::newRow("hex format") // value left intact
            << "0xFF"_ba << false << qint32(99) << qint32(99);
}

void tst_Helpers::fromByteArray_qint32()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(qint32, initialResult);
    QFETCH(qint32, expectedResult);

    qint32 result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_qint64_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<qint64>("initialResult"); // initial value of result before the call
    QTest::addColumn<qint64>("expectedResult");

    const quint64 overMax = static_cast<quint64>(std::numeric_limits<qint64>::max()) + 1;

    QTest::newRow("positive") << "42"_ba << true << qint64(0) << qint64(42);
    QTest::newRow("leading zeros") << "007"_ba << true << qint64(0) << qint64(7);
    QTest::newRow("max") << QByteArray::number(std::numeric_limits<qint64>::max())
                         << true << qint64(0) << std::numeric_limits<qint64>::max();
    QTest::newRow("min") << QByteArray::number(std::numeric_limits<qint64>::min())
                         << true << qint64(0) << std::numeric_limits<qint64>::min();
    QTest::newRow("overflow") // value left intact
            << QByteArray::number(overMax) << false << qint64(99) << qint64(99);
    QTest::newRow("not a number") // value left intact
            << "not_a_number"_ba << false << qint64(99) << qint64(99);
    QTest::newRow("empty") // value left intact
            << ""_ba << false << qint64(99) << qint64(99);
    QTest::newRow("decimal point") // value left intact
            << "42.5"_ba << false << qint64(99) << qint64(99);
    QTest::newRow("hex format") // value left intact
            << "0xFF"_ba << false << qint64(99) << qint64(99);
}

void tst_Helpers::fromByteArray_qint64()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(qint64, initialResult);
    QFETCH(qint64, expectedResult);

    qint64 result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_float_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<float>("initialResult"); // initial value of result before the call
    QTest::addColumn<float>("expectedResult");

    QTest::newRow("positive") << "1.5"_ba << true << 0.0f << 1.5f;
    QTest::newRow("negative") << "-2.5"_ba << true << 0.0f << -2.5f;
    QTest::newRow("max") << QByteArray::number(std::numeric_limits<float>::max())
            << true << 0.0f << std::numeric_limits<float>::max();
    QTest::newRow("min") << QByteArray::number(std::numeric_limits<float>::min())
            << true << 0.0f << std::numeric_limits<float>::min();
    QTest::newRow("-inf") // value left intact
            << "-inf"_ba << false << 99.0f << 99.0f;
    QTest::newRow("inf") // value left intact
            << "inf"_ba << false << 99.0f << 99.0f;
    QTest::newRow("nan") // value left intact
            << "nan"_ba << false << 99.0f << 99.0f;
    QTest::newRow("not a float") // value left intact
            << "not_a_float"_ba << false << 99.0f << 99.0f;
}

void tst_Helpers::fromByteArray_float()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(float, initialResult);
    QFETCH(float, expectedResult);

    float result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_double_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<double>("initialResult"); // initial value of result before the call
    QTest::addColumn<double>("expectedResult");

    QTest::newRow("positive") << "1.5"_ba << true << 0.0 << 1.5;
    QTest::newRow("negative") << "-2.5"_ba << true << 0.0 << -2.5;
    QTest::newRow("max")
            << QByteArray::number(std::numeric_limits<double>::max(), 'g',
                                  std::numeric_limits<double>::max_digits10)
            << true << 0.0 << std::numeric_limits<double>::max();
    QTest::newRow("min")
            << QByteArray::number(std::numeric_limits<double>::min(), 'g',
                                  std::numeric_limits<double>::max_digits10)
            << true << 0.0 << std::numeric_limits<double>::min();
    QTest::newRow("-inf") // value left intact
            << "-inf"_ba << false << 99.0 << 99.0;
    QTest::newRow("inf") // value left intact
            << "inf"_ba << false << 99.0 << 99.0;
    QTest::newRow("nan") // value left intact
            << "nan"_ba << false << 99.0 << 99.0;
    QTest::newRow("not a double") // value left intact
            << "not_a_double"_ba << false << 99.0 << 99.0;
}

void tst_Helpers::fromByteArray_double()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(double, initialResult);
    QFETCH(double, expectedResult);

    double result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result, expectedResult);
}

void tst_Helpers::fromByteArray_enum_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<TestEnum::eTestEnum>("expectedResultValue");
    QTest::addColumn<bool>("expectedResultValidity");

    QTest::newRow("valid VALUE_A") << "VALUE_A"_ba << true << TestEnum::eTestEnum::VALUE_A
                                   << true;
    QTest::newRow("valid VALUE_B") << "VALUE_B"_ba << true << TestEnum::eTestEnum::VALUE_B
                                   << true;
    QTest::newRow("UNKNOWN") << "UNKNOWN"_ba << false
                             << TestEnum::eTestEnum::INVALID_VALUE_OPENAPI_GENERATED << false;
}

void tst_Helpers::fromByteArray_enum()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(TestEnum::eTestEnum, expectedResultValue);
    QFETCH(bool, expectedResultValidity);

    TestEnum result;

    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result.isValid(), expectedResultValidity);
    QCOMPARE(result.getValue(), expectedResultValue);
}

void tst_Helpers::fromByteArray_object_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("expectedOk");
    QTest::addColumn<QOAIObject>("initialResult");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("expectedIsSet");

    QOAIObject empty;
    QOAIObject withKeyValue;
    withKeyValue.fromJson(R"({"key1":"value1"})"_L1);

    QTest::newRow("Not Json") << "not_json"_ba  << false << empty << QString("{}"_L1) << false;
    QTest::newRow("Valid JSON object") << R"({"key":"value"})"_ba  << true << empty
                                       << QString(R"({"key":"value"})"_L1) << true;
    QTest::newRow("JSON array not accepted") << "[1, 2, 3]"_ba << false << withKeyValue
                                             << QString(R"({"key1":"value1"})"_L1) << true;
                                             // value left intact
    QTest::newRow("Empty object") << "{}"_ba << true << withKeyValue << QString("{}"_L1) << false;
}

void tst_Helpers::fromByteArray_object()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(QOAIObject, initialResult);
    QFETCH(QString, expectedJson);
    QFETCH(bool, expectedIsSet);

    QOAIObject result = initialResult;
    const bool ok = fromByteArray(input, result);
    QCOMPARE(ok, expectedOk);
    QCOMPARE(result.isSet(), expectedIsSet);
    QCOMPARE(result.asJson(), expectedJson);
}

// bool fromByteArray(const QByteArray &input, QOAIHttpFileElement &value)
// This overload delegates directly to QOAIHttpFileElement::saveToLocalFile(),
// so testing it here is equivalent to testing that method directly.
void tst_Helpers::fromByteArray_httpFileElement()
{
    bool ok = false;
    QByteArray input = "file content"_ba;

    // Default-constructed file element has no path: writing fails
    QOAIHttpFileElement noFile;
    QVERIFY(!noFile.isSet()); // isSet checks the filename not the content.
    ok = fromByteArray(input, noFile);
    QCOMPARE(ok, false);
    QCOMPARE(noFile.asJson(), ""_L1);

    // File element with a valid file
    QTemporaryFile tmpFile;
    QVERIFY(tmpFile.open());
    tmpFile.write("content of temporary file :)");
    tmpFile.close();
    const QString tmpFilePath = tmpFile.fileName();

    QOAIHttpFileElement result(tmpFilePath);
    QVERIFY(result.isSet());
    QCOMPARE(result.asJson(), "content of temporary file :)"_L1);

    ok = fromByteArray(input, result);
    QCOMPARE(ok, true);
    QCOMPARE(result.asJson(), input);
}

void tst_Helpers::fromJsonValue_QDateTime_data()
{
    fromByteArray_QDateTime_data();
}

void tst_Helpers::fromJsonValue_QDateTime()
{
    QFETCH(QString, format);
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(QDateTime, initialResult);
    QFETCH(QDateTime, expectedResult);

    if (format.isEmpty())
        QVERIFY(setDateTimeFormat(Qt::ISODate));
    else if (format == "TextDate"_L1)
        QVERIFY(setDateTimeFormat(Qt::TextDate));
    else
        QVERIFY(setDateTimeFormat(format));
    const auto restoreFormat = qScopeGuard([] { setDateTimeFormat(Qt::ISODate); });

    QDateTime result = initialResult;
    const QJsonValue jsonVal(QString::fromUtf8(input));
    const bool ok = fromJsonValue(result, jsonVal);
    QCOMPARE(ok, expectedOk);
    QCOMPARE_EQ(result, expectedResult);
}

void tst_Helpers::fromJsonValue_QDateTime_nonStringInput()
{
    QDateTime result;
    QVERIFY(!fromJsonValue(result, QJsonValue()));
    QVERIFY(!fromJsonValue(result, QJsonValue(QJsonValue::Null)));

    const QDateTime initialValue(QDate(2026, 4, 27), QTime(12, 0, 0));
    result = initialValue;
    QVERIFY(!fromJsonValue(result, QJsonValue(42))); // not a string
    QCOMPARE_EQ(result, initialValue); // value left intact
}

void tst_Helpers::fromJsonValue_QDate_data()
{
    fromByteArray_QDate_data();
}

void tst_Helpers::fromJsonValue_QDate()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, expectedOk);
    QFETCH(QDate, initialResult);
    QFETCH(QDate, expectedResult);

    QDate result = initialResult;
    const QJsonValue jsonVal(QString::fromUtf8(input));
    const bool ok = fromJsonValue(result, jsonVal);
    QCOMPARE(ok, expectedOk);
    QCOMPARE_EQ(result, expectedResult);
}

void tst_Helpers::fromJsonValue_QDate_nonStringInput()
{
    QDate result;
    QVERIFY(!fromJsonValue(result, QJsonValue()));
    QVERIFY(!fromJsonValue(result, QJsonValue(QJsonValue::Null)));

    const QDate initialValue(2026, 4, 27);
    result = initialValue;
    QVERIFY(!fromJsonValue(result, QJsonValue(42)));
    QCOMPARE_EQ(result, initialValue); // value left intact
}

} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::tst_Helpers)
#include "tst_helpers.moc"
