// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"
#include "defaultapi.h"

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qset.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace TestData;

static QProcess serverProcess;
void startServerProcess()
{
    serverProcess.start(SERVER_PATH);
    if (!serverProcess.waitForStarted()) {
        qFatal() << "Couldn't start the server: " << serverProcess.errorString();
        exit(EXIT_FAILURE);
    }
    // give the process some time to properly start up the server
    QThread::currentThread()->msleep(1000);
}

class NestedData : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        if (serverProcess.state() != QProcess::ProcessState::Running)
            startServerProcess();
    }
    void deepNestedArray();
    void deepNestedMap();
    void cleanupTestCase();
};

void NestedData::deepNestedArray()
{
    QList<QList<QString>> requestBody = {{"Hello!!!", "Boo."}};
    bool done = false;
    DefaultApi api;
    api.submitDeepArray(requestBody, this, [&](const QRestReply &reply,
                                               const QSet<QList<QList<QString>>> &summary)
                        {
                            if ((done = reply.isSuccess())) {
                                const QSet<QList<QList<QString>>> expected{{{"Moin!!!"}}};
                                QCOMPARE(summary, expected);
                            }
                        });
    QTRY_COMPARE_EQ(done, true);
}

void NestedData::deepNestedMap()
{
    QMap<QString, QMap<QString, QMap<QString, QString>>> requestBody =
        {{ "level1", { { "level2", { { "level3", "value" } } } } }};
    bool done = false;
    DefaultApi api;
    api.submitDeepMap(requestBody, this, [&](const QRestReply &reply,
                                             const QMap<QString, QMap<QString, QString>> &summary)
                        {
                            if ((done = reply.isSuccess())) {
                                const QMap<QString, QMap<QString, QString>> expected
                                    = {{"level1", {{"level2", {"value"}}}}};
                                QCOMPARE(summary, expected);
                            }
                        });
    QTRY_COMPARE_EQ(done, true);
}

void NestedData::cleanupTestCase()
{
    if (serverProcess.state() == QProcess::ProcessState::Running) {
        serverProcess.kill();
        serverProcess.waitForFinished();
    }
}

QTEST_MAIN(NestedData)
#include "tst_nested.moc"
