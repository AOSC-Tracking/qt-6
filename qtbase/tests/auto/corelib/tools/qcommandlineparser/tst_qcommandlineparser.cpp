// Copyright (C) 2021 David Faure <faure@kde.org>
// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QTest>
#if QT_CONFIG(process)
#include <QProcess>
#endif
#include <QtCore/QCommandLineParser>

Q_DECLARE_METATYPE(char**)
Q_DECLARE_METATYPE(QCommandLineParser::OptionsAfterPositionalArgumentsMode)

class tst_QCommandLineParser : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();

private slots:
    void parsingModes_data();

    // In-process tests
    void testInvalidOptions();
    void testDuplicateOption();
    void testPositionalArguments();
    void testBooleanOption_data();
    void testBooleanOption();
    void testOptionsAndPositional_data();
    void testOptionsAndPositional();
    void testMultipleNames_data();
    void testMultipleNames();
    void testSingleValueOption_data();
    void testSingleValueOption();
    void testValueNotSet();
    void testMultipleValuesOption();
    void testUnknownOptionErrorHandling_data();
    void testUnknownOptionErrorHandling();
    void testDoubleDash_data();
    void testDoubleDash();
    void testDefaultValue();
    void testProcessNotCalled();
    void testEmptyArgsList();
    void testNoApplication();
    void testMissingOptionValue();
    void testStdinArgument_data();
    void testStdinArgument();
    void testSingleDashWordOptionModes_data();
    void testSingleDashWordOptionModes();
    void testCpp11StyleInitialization();

    // QProcess-based tests using qcommandlineparser_test_helper
    void testVersionOption();
    void testHelpOption_data();
    void testHelpOption();
    void testQuoteEscaping();
    void testUnknownOption();
    void testHelpAll_data();
    void testHelpAll();
    void testVeryLongOptionNames();
};

static char *empty_argv[] = { 0 };
static int empty_argc = 1;

void tst_QCommandLineParser::initTestCase()
{
    Q_ASSERT(!empty_argv[0]);
    empty_argv[0] = const_cast<char*>(QTest::currentAppName());
}

Q_DECLARE_METATYPE(QCommandLineParser::SingleDashWordOptionMode)

void tst_QCommandLineParser::parsingModes_data()
{
    QTest::addColumn<QCommandLineParser::SingleDashWordOptionMode>("parsingMode");

    QTest::newRow("collapsed") << QCommandLineParser::ParseAsCompactedShortOptions;
    QTest::newRow("implicitlylong") << QCommandLineParser::ParseAsLongOptions;
}

void tst_QCommandLineParser::testInvalidOptions()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineOption: Option names cannot start with a '-'");
    QVERIFY(!parser.addOption(QCommandLineOption(QStringLiteral("-v"), QStringLiteral("Displays version information."))));
}

void tst_QCommandLineParser::testDuplicateOption()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QVERIFY(parser.addOption(QCommandLineOption(QStringLiteral("h"), QStringLiteral("Hostname."), QStringLiteral("hostname"))));
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: already having an option named \"h\"");
    parser.addHelpOption();
}

void tst_QCommandLineParser::testPositionalArguments()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    const QString exeName = QCoreApplication::instance()->arguments().first(); // e.g. debug\tst_qcommandlineparser.exe on Windows
    QString expectedNoPositional =
            "Usage: " + exeName + "\n"
            "\n";

    QCOMPARE(parser.helpText(), expectedNoPositional);
    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "file.txt"));
    QCOMPARE(parser.positionalArguments(), QStringList() << QStringLiteral("file.txt"));

    parser.addPositionalArgument("file", "File names", "<foobar>");
    QString expected =
            "Usage: " + exeName + " <foobar>\n"
            "\n"
            "Arguments:\n"
            "  file File names\n";
}

void tst_QCommandLineParser::testBooleanOption_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expectedOptionNames");
    QTest::addColumn<bool>("expectedIsSet");

    QTest::newRow("set") << (QStringList() << "tst_qcommandlineparser" << "-b") << (QStringList() << "b") << true;
    QTest::newRow("unset") << (QStringList() << "tst_qcommandlineparser") << QStringList() << false;
}

void tst_QCommandLineParser::testBooleanOption()
{
    QFETCH(QStringList, args);
    QFETCH(QStringList, expectedOptionNames);
    QFETCH(bool, expectedIsSet);
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QVERIFY(parser.addOption(QCommandLineOption(QStringLiteral("b"))));
    QVERIFY(parser.parse(args));
    QCOMPARE(parser.optionNames(), expectedOptionNames);
    QCOMPARE(parser.isSet("b"), expectedIsSet);
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: option not expecting values: \"b\"");
    QCOMPARE(parser.values("b"), QStringList());
    QCOMPARE(parser.positionalArguments(), QStringList());
    // Should warn on typos
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: option not defined: \"c\"");
    QVERIFY(!parser.isSet("c"));
}

void tst_QCommandLineParser::testOptionsAndPositional_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expectedOptionNames");
    QTest::addColumn<bool>("expectedIsSet");
    QTest::addColumn<QStringList>("expectedPositionalArguments");
    QTest::addColumn<QCommandLineParser::OptionsAfterPositionalArgumentsMode>("parsingMode");

    const QStringList arg = QStringList() << "arg";
    QTest::newRow("before_positional_default") << (QStringList() << "tst_qcommandlineparser" << "-b" << "arg") << (QStringList() << "b") << true << arg << QCommandLineParser::ParseAsOptions;
    QTest::newRow("after_positional_default") << (QStringList() << "tst_qcommandlineparser" << "arg" << "-b") << (QStringList() << "b") << true << arg << QCommandLineParser::ParseAsOptions;
    QTest::newRow("before_positional_parseAsArg") << (QStringList() << "tst_qcommandlineparser" << "-b" << "arg") << (QStringList() << "b") << true << arg << QCommandLineParser::ParseAsPositionalArguments;
    QTest::newRow("after_positional_parseAsArg") << (QStringList() << "tst_qcommandlineparser" << "arg" << "-b") << (QStringList()) << false << (QStringList() << "arg" << "-b") << QCommandLineParser::ParseAsPositionalArguments;
}

void tst_QCommandLineParser::testOptionsAndPositional()
{
    QFETCH(QStringList, args);
    QFETCH(QStringList, expectedOptionNames);
    QFETCH(bool, expectedIsSet);
    QFETCH(QStringList, expectedPositionalArguments);
    QFETCH(QCommandLineParser::OptionsAfterPositionalArgumentsMode, parsingMode);

    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    const QString exeName = QCoreApplication::instance()->arguments().first(); // e.g. debug\tst_qcommandlineparser.exe on Windows
    const QString expectedHelpText =
            "Usage: " + exeName + " [options] input\n"
            "\n"
            "Options:\n"
            "  -b     a boolean option\n"
            "\n"
            "Arguments:\n"
            "  input  File names\n";

    parser.setOptionsAfterPositionalArgumentsMode(parsingMode);
    parser.addPositionalArgument("input", "File names");
    QVERIFY(parser.addOption(QCommandLineOption(QStringLiteral("b"), QStringLiteral("a boolean option"))));
    QVERIFY(parser.parse(args));
    QCOMPARE(parser.optionNames(), expectedOptionNames);
    QCOMPARE(parser.isSet("b"), expectedIsSet);
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: option not expecting values: \"b\"");
    QCOMPARE(parser.values("b"), QStringList());
    QCOMPARE(parser.positionalArguments(), expectedPositionalArguments);
    QCOMPARE(parser.helpText(), expectedHelpText);
}

void tst_QCommandLineParser::testMultipleNames_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expectedOptionNames");

    QTest::newRow("short") << (QStringList() << "tst_qcommandlineparser" << "-v") << (QStringList() << "v");
    QTest::newRow("long") << (QStringList() << "tst_qcommandlineparser" << "--version") << (QStringList() << "version");
    QTest::newRow("not_set") << (QStringList() << "tst_qcommandlineparser") << QStringList();
}

void tst_QCommandLineParser::testMultipleNames()
{
    QFETCH(QStringList, args);
    QFETCH(QStringList, expectedOptionNames);
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineOption option(QStringList() << "v" << "version", QStringLiteral("Show version information"));
    QCOMPARE(option.names(), QStringList() << "v" << "version");
    QCommandLineParser parser;
    QVERIFY(parser.addOption(option));
    QVERIFY(parser.parse(args));
    QCOMPARE(parser.optionNames(), expectedOptionNames);
    const bool expectedIsSet = !expectedOptionNames.isEmpty();
    QCOMPARE(parser.isSet("v"), expectedIsSet);
    QCOMPARE(parser.isSet("version"), expectedIsSet);
}

void tst_QCommandLineParser::testSingleValueOption_data()
{
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("defaults");
    QTest::addColumn<bool>("expectedIsSet");

    QTest::newRow("short") << (QStringList() << "tst" << "-s" << "oxygen") << QStringList() << true;
    QTest::newRow("long") << (QStringList() << "tst" << "--style" << "oxygen") << QStringList() << true;
    QTest::newRow("longequal") << (QStringList() << "tst" << "--style=oxygen") << QStringList() << true;
    QTest::newRow("default") << (QStringList() << "tst") << (QStringList() << "oxygen") << false;
}

void tst_QCommandLineParser::testSingleValueOption()
{
    QFETCH(QStringList, args);
    QFETCH(QStringList, defaults);
    QFETCH(bool, expectedIsSet);
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QCommandLineOption option(QStringList() << "s" << "style", QStringLiteral("style name"), "styleName");
    option.setDefaultValues(defaults);
    QVERIFY(parser.addOption(option));
    for (int mode = 0; mode < 2; ++mode) {
        parser.setSingleDashWordOptionMode(QCommandLineParser::SingleDashWordOptionMode(mode));
        QVERIFY(parser.parse(args));
        QCOMPARE(parser.isSet("s"), expectedIsSet);
        QCOMPARE(parser.isSet("style"), expectedIsSet);
        QCOMPARE(parser.isSet(option), expectedIsSet);
        QCOMPARE(parser.value("s"), QString("oxygen"));
        QCOMPARE(parser.value("style"), QString("oxygen"));
        QCOMPARE(parser.values("s"), QStringList() << "oxygen");
        QCOMPARE(parser.values("style"), QStringList() << "oxygen");
        QCOMPARE(parser.values(option), QStringList() << "oxygen");
        QCOMPARE(parser.positionalArguments(), QStringList());
    }
    // Should warn on typos
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: option not defined: \"c\"");
    QVERIFY(parser.values("c").isEmpty());
}

void tst_QCommandLineParser::testValueNotSet()
{
    QCoreApplication app(empty_argc, empty_argv);
    // Not set, no default value
    QCommandLineParser parser;
    QCommandLineOption option(QStringList() << "s" << "style", QStringLiteral("style name"));
    option.setValueName("styleName");
    QVERIFY(parser.addOption(option));
    QVERIFY(parser.parse(QStringList() << "tst"));
    QCOMPARE(parser.optionNames(), QStringList());
    QVERIFY(!parser.isSet("s"));
    QVERIFY(!parser.isSet("style"));
    QCOMPARE(parser.value("s"), QString());
    QCOMPARE(parser.value("style"), QString());
    QCOMPARE(parser.values("s"), QStringList());
    QCOMPARE(parser.values("style"), QStringList());
}

void tst_QCommandLineParser::testMultipleValuesOption()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineOption option(QStringLiteral("param"), QStringLiteral("Pass parameter to the backend."));
    option.setValueName("key=value");
    QCommandLineParser parser;
    QVERIFY(parser.addOption(option));
    {
        QVERIFY(parser.parse(QStringList() << "tst" << "--param" << "key1=value1"));
        QVERIFY(parser.isSet("param"));
        QCOMPARE(parser.values("param"), QStringList() << "key1=value1");
        QCOMPARE(parser.value("param"), QString("key1=value1"));
    }
    {
        QVERIFY(parser.parse(QStringList() << "tst" << "--param" << "key1=value1" << "--param" << "key2=value2"));
        QVERIFY(parser.isSet("param"));
        QCOMPARE(parser.values("param"), QStringList() << "key1=value1" << "key2=value2");
        QCOMPARE(parser.value("param"), QString("key2=value2"));
    }

    QString expected =
        "Usage: tst_qcommandlineparser [options]\n"
        "\n"
        "Options:\n"
        "  --param <key=value>  Pass parameter to the backend.\n";

    const QString exeName = QCoreApplication::instance()->arguments().first(); // e.g. debug\tst_qcommandlineparser.exe on Windows
    expected.replace(QStringLiteral("tst_qcommandlineparser"), exeName);
    QCOMPARE(parser.helpText(), expected);
}

void tst_QCommandLineParser::testUnknownOptionErrorHandling_data()
{
    QTest::addColumn<QCommandLineParser::SingleDashWordOptionMode>("parsingMode");
    QTest::addColumn<QStringList>("args");
    QTest::addColumn<QStringList>("expectedUnknownOptionNames");
    QTest::addColumn<QString>("expectedErrorText");

    const QStringList args_hello = QStringList() << "tst_qcommandlineparser" << "--hello";
    const QString error_hello("Unknown option 'hello'.");
    QTest::newRow("unknown_name_collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << args_hello << QStringList("hello") << error_hello;
    QTest::newRow("unknown_name_long") << QCommandLineParser::ParseAsLongOptions << args_hello << QStringList("hello") << error_hello;

    const QStringList args_value = QStringList() << "tst_qcommandlineparser" << "-b=1";
    QTest::newRow("bool_with_value_collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << args_value << QStringList() << QString("Unexpected value after '-b'.");
    QTest::newRow("bool_with_value_long") << QCommandLineParser::ParseAsLongOptions << args_value << QStringList() << QString("Unexpected value after '-b'.");

    const QStringList args_dash_long = QStringList() << "tst_qcommandlineparser" << "-bool";
    const QString error_bool("Unknown options: o, o, l.");
    QTest::newRow("unknown_name_long_collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << args_dash_long << (QStringList() << "o" << "o" << "l") << error_bool;
}

void tst_QCommandLineParser::testUnknownOptionErrorHandling()
{
    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);
    QFETCH(QStringList, args);
    QFETCH(QStringList, expectedUnknownOptionNames);
    QFETCH(QString, expectedErrorText);

    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(parsingMode);
    QVERIFY(parser.addOption(QCommandLineOption(QStringList() << "b" << "bool", QStringLiteral("a boolean option"))));
    QCOMPARE(parser.parse(args), expectedErrorText.isEmpty());
    QCOMPARE(parser.unknownOptionNames(), expectedUnknownOptionNames);
    QCOMPARE(parser.errorText(), expectedErrorText);
}

void tst_QCommandLineParser::testDoubleDash_data()
{
    parsingModes_data();
}

void tst_QCommandLineParser::testDoubleDash()
{
    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);

    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QVERIFY(parser.addOption(QCommandLineOption(QStringList() << "o" << "output", QStringLiteral("Output file"), QStringLiteral("filename"))));
    parser.setSingleDashWordOptionMode(parsingMode);
    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "--output" << "foo"));
    QCOMPARE(parser.value("output"), QString("foo"));
    QCOMPARE(parser.positionalArguments(), QStringList());
    QCOMPARE(parser.unknownOptionNames(), QStringList());
    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "--" << "--output" << "bar" << "-b" << "bleh"));
    QCOMPARE(parser.value("output"), QString());
    QCOMPARE(parser.positionalArguments(), QStringList() << "--output" << "bar" << "-b" << "bleh");
    QCOMPARE(parser.unknownOptionNames(), QStringList());
}

void tst_QCommandLineParser::testDefaultValue()
{
    QCommandLineOption opt(QStringLiteral("name"), QStringLiteral("desc"),
                           QStringLiteral("valueName"), QStringLiteral("default"));
    QCOMPARE(opt.defaultValues(), QStringList(QStringLiteral("default")));
    opt.setDefaultValue(QStringLiteral(""));
    QCOMPARE(opt.defaultValues(), QStringList());
    opt.setDefaultValue(QStringLiteral("default"));
    QCOMPARE(opt.defaultValues(), QStringList(QStringLiteral("default")));
}

void tst_QCommandLineParser::testProcessNotCalled()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QVERIFY(parser.addOption(QCommandLineOption(QStringLiteral("b"), QStringLiteral("a boolean option"))));
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: call process() or parse() before isSet");
    QVERIFY(!parser.isSet("b"));
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: call process() or parse() before values");
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: option not expecting values: \"b\"");
    QCOMPARE(parser.values("b"), QStringList());
}

void tst_QCommandLineParser::testEmptyArgsList()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QTest::ignoreMessage(QtWarningMsg, "QCommandLineParser: argument list cannot be empty, it should contain at least the executable name");
    QVERIFY(!parser.parse(QStringList())); // invalid call, argv[0] is missing
}

void tst_QCommandLineParser::testNoApplication()
{
    QCommandLineOption option(QStringLiteral("param"), QStringLiteral("Pass parameter to the backend."));
    option.setValueName("key=value");
    QCommandLineParser parser;
    QVERIFY(parser.addOption(option));
    {
        QVERIFY(parser.parse(QStringList() << "tst" << "--param" << "key1=value1"));
        QVERIFY(parser.isSet("param"));
        QCOMPARE(parser.values("param"), QStringList() << "key1=value1");
        QCOMPARE(parser.value("param"), QString("key1=value1"));
    }
    {
        QVERIFY(parser.parse(QStringList() << "tst" << "--param" << "key1=value1" << "--param" << "key2=value2"));
        QVERIFY(parser.isSet("param"));
        QCOMPARE(parser.values("param"), QStringList() << "key1=value1" << "key2=value2");
        QCOMPARE(parser.value("param"), QString("key2=value2"));
    }

    const QString expected =
        "Usage: <executable_name> [options]\n"
        "\n"
        "Options:\n"
        "  --param <key=value>  Pass parameter to the backend.\n";

    QCOMPARE(parser.helpText(), expected);
}

void tst_QCommandLineParser::testMissingOptionValue()
{
    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    QVERIFY(parser.addOption(QCommandLineOption(QStringLiteral("option"), QStringLiteral("An option"), "value")));
    QVERIFY(!parser.parse(QStringList() << "argv0" << "--option")); // the user forgot to pass a value for --option
    QCOMPARE(parser.value("option"), QString());
    QCOMPARE(parser.errorText(), QString("Missing value after '--option'."));
}

void tst_QCommandLineParser::testStdinArgument_data()
{
    parsingModes_data();
}

void tst_QCommandLineParser::testStdinArgument()
{
    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);

    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(parsingMode);
    QVERIFY(parser.addOption(QCommandLineOption(QStringList() << "i" << "input", QStringLiteral("Input file."), QStringLiteral("filename"))));
    QVERIFY(parser.addOption(QCommandLineOption("b", QStringLiteral("Boolean option."))));
    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "--input" << "-"));
    QCOMPARE(parser.value("input"), QString("-"));
    QCOMPARE(parser.positionalArguments(), QStringList());
    QCOMPARE(parser.unknownOptionNames(), QStringList());

    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "--input" << "-" << "-b" << "arg"));
    QCOMPARE(parser.value("input"), QString("-"));
    QVERIFY(parser.isSet("b"));
    QCOMPARE(parser.positionalArguments(), QStringList() << "arg");
    QCOMPARE(parser.unknownOptionNames(), QStringList());

    QVERIFY(parser.parse(QStringList() << "tst_qcommandlineparser" << "-"));
    QCOMPARE(parser.value("input"), QString());
    QVERIFY(!parser.isSet("b"));
    QCOMPARE(parser.positionalArguments(), QStringList() << "-");
    QCOMPARE(parser.unknownOptionNames(), QStringList());
}

void tst_QCommandLineParser::testSingleDashWordOptionModes_data()
{
    QTest::addColumn<QCommandLineParser::SingleDashWordOptionMode>("parsingMode");
    QTest::addColumn<QStringList>("commandLine");
    QTest::addColumn<QStringList>("expectedOptionNames");
    QTest::addColumn<QStringList>("expectedOptionValues");
    QTest::addColumn<QStringList>("invalidOptionValues");

    QTest::newRow("collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << (QStringList() << "-abc" << "val")
                               << (QStringList() << "a" << "b" << "c") << (QStringList() << QString() << QString() << "val")
                               << (QStringList() << "a" << "b");
    QTest::newRow("collapsed_with_equalsign_value") << QCommandLineParser::ParseAsCompactedShortOptions << (QStringList() << "-abc=val")
                               << (QStringList() << "a" << "b" << "c") << (QStringList() << QString() << QString() << "val")
                               << (QStringList() << "a" << "b");
    QTest::newRow("collapsed_explicit_longoption") << QCommandLineParser::ParseAsCompactedShortOptions << QStringList("--nn")
                               << QStringList("nn") << QStringList() << QStringList();
    QTest::newRow("collapsed_longoption_value") << QCommandLineParser::ParseAsCompactedShortOptions << (QStringList() << "--abc" << "val")
                               << QStringList("abc") << QStringList("val") << QStringList();
    QTest::newRow("compiler")  << QCommandLineParser::ParseAsCompactedShortOptions << QStringList("-cab")
                               << QStringList("c") << QStringList("ab") << QStringList();
    QTest::newRow("compiler_with_space") << QCommandLineParser::ParseAsCompactedShortOptions << (QStringList() << "-c" << "val")
                               << QStringList("c") << QStringList("val") << QStringList();

    QTest::newRow("implicitlylong") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "-abc" << "val")
                               << QStringList("abc") << QStringList("val") << QStringList();
    QTest::newRow("implicitlylong_equal") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "-abc=val")
                               << QStringList("abc") << QStringList("val") << QStringList();
    QTest::newRow("implicitlylong_longoption") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "--nn")
                               << QStringList("nn") << QStringList() << QStringList();
    QTest::newRow("implicitlylong_longoption_value") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "--abc" << "val")
                               << QStringList("abc") << QStringList("val") << QStringList();
    QTest::newRow("implicitlylong_with_space") << QCommandLineParser::ParseAsCompactedShortOptions << (QStringList() << "-c" << "val")
                               << QStringList("c") << QStringList("val") << QStringList();

    QTest::newRow("forceshort_detached") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "-I" << "45")
                               << QStringList("I") << QStringList("45") << QStringList();
    QTest::newRow("forceshort_attached") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "-I46")
                               << QStringList("I") << QStringList("46") << QStringList();
    QTest::newRow("forceshort_mixed") << QCommandLineParser::ParseAsLongOptions << (QStringList() << "-I45" << "-nn")
                               << (QStringList() << "I" << "nn") << QStringList("45") << QStringList();
}

void tst_QCommandLineParser::testSingleDashWordOptionModes()
{
    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);
    QFETCH(QStringList, commandLine);
    QFETCH(QStringList, expectedOptionNames);
    QFETCH(QStringList, expectedOptionValues);
    QFETCH(QStringList, invalidOptionValues);

    commandLine.prepend("tst_QCommandLineParser");

    QCoreApplication app(empty_argc, empty_argv);
    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(parsingMode);
    QVERIFY(parser.addOption(QCommandLineOption("a", QStringLiteral("a option."))));
    QVERIFY(parser.addOption(QCommandLineOption("b", QStringLiteral("b option."))));
    QVERIFY(parser.addOption(QCommandLineOption(QStringList() << "c" << "abc", QStringLiteral("c option."), QStringLiteral("value"))));
    QVERIFY(parser.addOption(QCommandLineOption("nn", QStringLiteral("nn option."))));
    QCommandLineOption forceShort(QStringLiteral("I"), QStringLiteral("always short option"),
                                  QStringLiteral("path"), QStringLiteral("default"));
    forceShort.setFlags(QCommandLineOption::ShortOptionStyle);
    QVERIFY(parser.addOption(forceShort));
    QVERIFY(parser.parse(commandLine));
    QCOMPARE(parser.optionNames(), expectedOptionNames);
    for (int i = 0; i < expectedOptionValues.size(); ++i) {
        const QString option = parser.optionNames().at(i);
        if (invalidOptionValues.contains(option)) {
            QByteArray msg = QLatin1String("QCommandLineParser: option not expecting values: \"%1\"").arg(option).toLatin1();
            QTest::ignoreMessage(QtWarningMsg, msg.data());
        }
        QCOMPARE(parser.value(option), expectedOptionValues.at(i));
    }
    QCOMPARE(parser.unknownOptionNames(), QStringList());
}

void tst_QCommandLineParser::testCpp11StyleInitialization()
{
    QCoreApplication app(empty_argc, empty_argv);

    QCommandLineParser parser;
    // primarily check that this compiles:
    QVERIFY(parser.addOptions({
        { "a",                "The A option." },
        { { "v", "verbose" }, "The verbose option." },
        { { "i", "infile" },  "The input file.", "value" },
    }));
    // but do a very basic functionality test, too:
    QVERIFY(parser.parse({"tst_QCommandLineParser", "-a", "-vvv", "--infile=in.txt"}));
    QCOMPARE(parser.optionNames(), (QStringList{"a", "v", "v", "v", "infile"}));
    QCOMPARE(parser.value("infile"), QString("in.txt"));
}

void tst_QCommandLineParser::testVersionOption()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#elif defined(Q_OS_ANDROID)
    QSKIP("Deploying executable applications to file system on Android not supported.");
#else

    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() << "0" << "--version");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QString output = process.readAll();
#ifdef Q_OS_WIN
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
#endif
    QCOMPARE(output, QString("qcommandlineparser_test_helper 1.0\n"));
#endif // QT_CONFIG(process)
}

static const char expectedOptionsHelp[] =
        "Options:\n"
        "  -h, --help                  Displays help on commandline options.\n"
        "  --help-all                  Displays help, including generic Qt options.\n"
        "  -v, --version               Displays version information.\n"
        "  --load <url>                Load file from URL.\n"
        "  -o, --output <file>         Set output file.\n"
        "  -D <key=value>              Define macro.\n"
        "  --long-option\n"
        "  -n, --no-implicit-includes  Disable magic generation of implicit\n"
        "                              #include-directives.\n"
        "  --newline                   This is an option with a rather long\n"
        "                              description using explicit newline characters (but\n"
        "                              testing automatic wrapping too). In addition,\n"
        "                              here, we test breaking after a comma. Testing\n"
        "                              -option. Long URL:\n"
        "                              http://qt-project.org/wiki/How_to_create_a_library\n"
        "                              _with_Qt_and_use_it_in_an_application\n"
        "                              abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx\n"
        "                              yzabcdefghijklmnopqrstuvwxyz\n";

void tst_QCommandLineParser::testHelpOption_data()
{
    QTest::addColumn<QCommandLineParser::SingleDashWordOptionMode>("parsingMode");
    QTest::addColumn<QString>("expectedHelpOutput");

    QString expectedOutput = QString::fromLatin1(
        "Usage: testhelper/qcommandlineparser_test_helper [options] parsingMode command\n"
        "Test helper\n"
        "\n")
        + QString::fromLatin1(expectedOptionsHelp) +
        QString::fromLatin1(
        "\n"
        "Arguments:\n"
        "  parsingMode                 The parsing mode to test.\n"
        "  command                     The command to execute.\n");
#ifdef Q_OS_WIN
    expectedOutput.replace("  -h, --help                  Displays help on commandline options.\n",
                           "  -?, -h, --help              Displays help on commandline options.\n");
    expectedOutput.replace("testhelper/", "testhelper\\");
#endif

    QTest::newRow("collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << expectedOutput;
    QTest::newRow("long")      << QCommandLineParser::ParseAsLongOptions << expectedOutput;
}

void tst_QCommandLineParser::testHelpOption()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#elif defined(Q_OS_ANDROID)
    QSKIP("Deploying executable applications to file system on Android not supported.");
#else

    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);
    QFETCH(QString, expectedHelpOutput);
    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() << QString::number(parsingMode) << "--help");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QString output = process.readAll();
#ifdef Q_OS_WIN
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
#endif
    QCOMPARE(output.split('\n'), expectedHelpOutput.split('\n')); // easier to debug than the next line, on failure
    QCOMPARE(output, expectedHelpOutput);

    process.start("testhelper/qcommandlineparser_test_helper", QStringList() << "0" << "resize" << "--help");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    output = process.readAll();
#ifdef Q_OS_WIN
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
#endif
    QByteArray expectedResizeHelp = QByteArrayLiteral(
        "Usage: testhelper/qcommandlineparser_test_helper [options] resize [resize_options]\n"
        "Test helper\n"
        "\n")
        + expectedOptionsHelp +
        "  --size <size>               New size.\n"
        "\n"
        "Arguments:\n"
        "  resize                      Resize the object to a new size.\n";
#ifdef Q_OS_WIN
    expectedResizeHelp.replace("  -h, --help                  Displays help on commandline options.\n",
                               "  -?, -h, --help              Displays help on commandline options.\n");
    expectedResizeHelp.replace("testhelper/", "testhelper\\");
#endif
    QCOMPARE(output, QString(expectedResizeHelp));
#endif // QT_CONFIG(process)
}

void tst_QCommandLineParser::testQuoteEscaping()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#elif defined(Q_OS_ANDROID)
    QSKIP("Deploying executable applications to file system on Android not supported.");
#else
    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() <<
            QString::number(QCommandLineParser::ParseAsCompactedShortOptions) <<
            "\\\\server\\path" <<
            "-DKEY1=\"VALUE1\""
            "-DQTBUG-15379=C:\\path\\'file.ext" <<
            "-DQTBUG-30628=C:\\temp\\'file'.ext");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QString output = process.readAll();
    QVERIFY2(!output.contains("ERROR"), qPrintable(output));
    QVERIFY2(output.contains("\\\\server\\path"), qPrintable(output));
    QVERIFY2(output.contains("KEY1=\"VALUE1\""), qPrintable(output));
    QVERIFY2(output.contains("QTBUG-15379=C:\\path\\'file.ext"), qPrintable(output));
    QVERIFY2(output.contains("QTBUG-30628=C:\\temp\\'file'.ext"), qPrintable(output));
#endif // QT_CONFIG(process)
}

void tst_QCommandLineParser::testUnknownOption()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#elif defined(Q_OS_ANDROID)
    QSKIP("Deploying executable applications to file system on Android not supported.");
#else
    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() <<
            QString::number(QCommandLineParser::ParseAsLongOptions) <<
            "-unknown-option");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    process.setReadChannel(QProcess::StandardError);
    QString output = process.readAll();
    QVERIFY2(output.contains("qcommandlineparser_test_helper"), qPrintable(output)); // separate in case of .exe extension
    QVERIFY2(output.contains(": Unknown option 'unknown-option'"), qPrintable(output));
#endif // QT_CONFIG(process)
}

void tst_QCommandLineParser::testHelpAll_data()
{
    QTest::addColumn<QCommandLineParser::SingleDashWordOptionMode>("parsingMode");
    QTest::addColumn<QString>("expectedHelpOutput");

    QString expectedOutput = QString::fromLatin1(
        "Usage: testhelper/qcommandlineparser_test_helper [options] parsingMode command\n"
        "Test helper\n"
        "\n")
        + QString::fromLatin1(expectedOptionsHelp) +
        QString::fromLatin1(
        "  --qmljsdebugger <value>     Activates the QML/JS debugger with a specified\n"
        "                              port. The value must be of format\n"
        "                              port:1234[,block]. \"block\" makes the application\n"
        "                              wait for a connection.\n"
        "\n"
        "Arguments:\n"
        "  parsingMode                 The parsing mode to test.\n"
        "  command                     The command to execute.\n");
#ifdef Q_OS_WIN
    expectedOutput.replace("  -h, --help                  Displays help on commandline options.\n",
                           "  -?, -h, --help              Displays help on commandline options.\n");
    expectedOutput.replace("testhelper/", "testhelper\\");
#endif

    QTest::newRow("collapsed") << QCommandLineParser::ParseAsCompactedShortOptions << expectedOutput;
    QTest::newRow("long")      << QCommandLineParser::ParseAsLongOptions << expectedOutput;
}

void tst_QCommandLineParser::testHelpAll()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#else
#ifdef Q_OS_ANDROID
    QSKIP("Deploying executable applications to file system on Android not supported.");
#endif

    QFETCH(QCommandLineParser::SingleDashWordOptionMode, parsingMode);
    QFETCH(QString, expectedHelpOutput);
    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() << QString::number(parsingMode) << "--help-all");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QString output = process.readAll();
#ifdef Q_OS_WIN
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
#endif
    QCOMPARE(output.split('\n'), expectedHelpOutput.split('\n')); // easier to debug than the next line, on failure
    QCOMPARE(output, expectedHelpOutput);
#endif // QT_CONFIG(process)
}

void tst_QCommandLineParser::testVeryLongOptionNames()
{
#if !QT_CONFIG(process)
    QSKIP("This test requires QProcess support");
#elif defined(Q_OS_ANDROID)
    QSKIP("Deploying executable applications to file system on Android not supported.");
#else

    QCoreApplication app(empty_argc, empty_argv);
    QProcess process;
    process.start("testhelper/qcommandlineparser_test_helper", QStringList() << "0" << "long" << "--help");
    QVERIFY(process.waitForFinished(5000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QString output = process.readAll();
#ifdef Q_OS_WIN
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
#endif
    const QStringList lines = output.split('\n');
    const int last = lines.size() - 1;
    // Let's not compare everything, just the final parts.
    QCOMPARE(lines.at(last - 7), "                                                     cdefghijklmnopqrstuvwxyz");
    QCOMPARE(lines.at(last - 6), "  --looooooooooooong-option, --looooong-opt-alias <l Short description");
    QCOMPARE(lines.at(last - 5), "  ooooooooooooong-value-name>");
    QCOMPARE(lines.at(last - 4), "");
    QCOMPARE(lines.at(last - 3), "Arguments:");
    QCOMPARE(lines.at(last - 2), "  parsingMode                                        The parsing mode to test.");
    QCOMPARE(lines.at(last - 1), "  command                                            The command to execute.");

#endif // QT_CONFIG(process)
}

QTEST_APPLESS_MAIN(tst_QCommandLineParser)
#include "tst_qcommandlineparser.moc"

