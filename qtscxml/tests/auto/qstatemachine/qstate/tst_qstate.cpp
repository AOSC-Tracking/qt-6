// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include "qeventtransition.h"
#include "qhistorystate.h"
#include "qstate.h"
#include "qstatemachine.h"
#include "qsignaltransition.h"

class tst_QState : public QObject
{
    Q_OBJECT

private slots:
    void assignProperty();
    void assignPropertyTwice();
    void historyInitialState();
    void transitions();
    void privateSignals();
    void parallelStateAndInitialState();
};

class TestClass: public QObject
{
    Q_OBJECT
public:
    TestClass() : called(false) {}
    bool called;

public slots:
    void slot() { called = true; }


};

void tst_QState::assignProperty()
{
    QStateMachine machine;

    QObject object;
    object.setProperty("fooBar", 10);

    QState *s1 = new QState(&machine);
    s1->assignProperty(&object, "fooBar", 20);

    machine.setInitialState(s1);
    machine.start();

    QTRY_COMPARE(object.property("fooBar").toInt(), 20);
}

void tst_QState::assignPropertyTwice()
{
    QStateMachine machine;

    QObject object;
    object.setProperty("fooBar", 10);

    QState *s1 = new QState(&machine);
    s1->assignProperty(&object, "fooBar", 20);
    s1->assignProperty(&object, "fooBar", 30);

    machine.setInitialState(s1);
    machine.start();

    QTRY_COMPARE(object.property("fooBar").toInt(), 30);
}

class EventTestTransition: public QAbstractTransition
{
public:
    EventTestTransition(QEvent::Type type, QState *targetState)
        : QAbstractTransition(), m_type(type)
    {
        setTargetState(targetState);
    }

protected:
    bool eventTest(QEvent *e) override
    {
        return e->type() == m_type;
    }

    void onTransition(QEvent *) override {}

private:
    QEvent::Type m_type;

};

void tst_QState::historyInitialState()
{
    QStateMachine machine;

    QState *s1 = new QState(&machine);

    QState *s2 = new QState(&machine);
    QHistoryState *h1 = new QHistoryState(s2);

    s2->setInitialState(h1);

    QState *s3 = new QState(s2);
    h1->setDefaultState(s3);

    QState *s4 = new QState(s2);

    s1->addTransition(new EventTestTransition(QEvent::User, s2));
    s2->addTransition(new EventTestTransition(QEvent::User, s1));
    s3->addTransition(new EventTestTransition(QEvent::Type(QEvent::User+1), s4));

    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 1);
    QTRY_VERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 2);
    QTRY_VERIFY(machine.configuration().contains(s2));
    QTRY_VERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 1);
    QTRY_VERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 2);
    QTRY_VERIFY(machine.configuration().contains(s2));
    QTRY_VERIFY(machine.configuration().contains(s3));

    machine.postEvent(new QEvent(QEvent::Type(QEvent::User+1)));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 2);
    QTRY_VERIFY(machine.configuration().contains(s2));
    QTRY_VERIFY(machine.configuration().contains(s4));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 1);
    QTRY_VERIFY(machine.configuration().contains(s1));

    machine.postEvent(new QEvent(QEvent::User));
    QCoreApplication::processEvents();

    QTRY_COMPARE(machine.configuration().size(), 2);
    QTRY_VERIFY(machine.configuration().contains(s2));
    QTRY_VERIFY(machine.configuration().contains(s4));
}

void tst_QState::transitions()
{
    QState s1;
    QState s2;

    QVERIFY(s1.transitions().isEmpty());

    QAbstractTransition *t1 = s1.addTransition(this, SIGNAL(destroyed()), &s2);
    QAbstractTransition *t1_1 = s1.addTransition(this, &tst_QState::destroyed, &s2);
    QVERIFY(t1 != 0);
    QVERIFY(t1_1 != 0);
    QCOMPARE(s1.transitions().size(), 2);
    QCOMPARE(s1.transitions().first(), t1);
    QCOMPARE(s1.transitions().last(), t1_1);
    QVERIFY(s2.transitions().isEmpty());

    s1.removeTransition(t1);
    s1.removeTransition(t1_1); // Releases ownership!
    delete t1_1;               // So, delete manually.
    QVERIFY(s1.transitions().isEmpty());

    s1.addTransition(t1);
    QCOMPARE(s1.transitions().size(), 1);
    QCOMPARE(s1.transitions().first(), t1);

    QAbstractTransition *t2 = new QEventTransition(&s1);
    QCOMPARE(s1.transitions().size(), 2);
    QVERIFY(s1.transitions().contains(t1));
    QVERIFY(s1.transitions().contains(t2));

    // Transitions from child states should not be reported.
    QState *s21 = new QState(&s2);
    QAbstractTransition *t3 = s21->addTransition(this, SIGNAL(destroyed()), &s2);
    QVERIFY(s2.transitions().isEmpty());
    QCOMPARE(s21->transitions().size(), 1);
    QCOMPARE(s21->transitions().first(), t3);
}

class MyState : public QState
{
    Q_OBJECT
public:
    MyState(QState *parent = 0)
      : QState(parent)
    {

    }

    void emitPrivateSignals()
    {
        // These deliberately do not compile
//         emit entered();
//         emit exited();
//
//         emit entered(QPrivateSignal());
//         emit exited(QPrivateSignal());
//
//         emit entered(QAbstractState::QPrivateSignal());
//         emit exited(QAbstractState::QPrivateSignal());
    }

};

class MyTransition : public QSignalTransition
{
    Q_OBJECT
public:
    MyTransition(QObject * sender, const char * signal, QState *sourceState = 0)
      : QSignalTransition(sender, signal, sourceState)
    {

    }

    void emitPrivateSignals()
    {
        // These deliberately do not compile
//         emit triggered();
//
//         emit triggered(QPrivateSignal());
//
//         emit triggered(QAbstractTransition::QPrivateSignal());
    }
};

class SignalConnectionTester : public QObject
{
    Q_OBJECT
public:
    SignalConnectionTester(QObject *parent = 0)
      : QObject(parent), testPassed(false)
    {

    }

public Q_SLOTS:
    void testSlot()
    {
      testPassed = true;
    }

public:
    bool testPassed;
};

class TestTrigger : public QObject
{
  Q_OBJECT
public:
    TestTrigger(QObject *parent = 0)
      : QObject(parent)
    {

    }

    void emitTrigger()
    {
        emit trigger();
    }

signals:
    void trigger();
};

void tst_QState::privateSignals()
{
    QStateMachine machine;

    QState *s1 = new QState(&machine);
    MyState *s2 = new MyState(&machine);

    TestTrigger testTrigger;

    MyTransition *t1 = new MyTransition(&testTrigger, SIGNAL(trigger()), s1);
    s1->addTransition(t1);
    t1->setTargetState(s2);

    machine.setInitialState(s1);
    machine.start();
    QCoreApplication::processEvents();

    SignalConnectionTester s1Tester;
    SignalConnectionTester s2Tester;
    SignalConnectionTester t1Tester;

    QObject::connect(s1, &QState::exited, &s1Tester, &SignalConnectionTester::testSlot);
    QObject::connect(s2, &QState::entered, &s2Tester, &SignalConnectionTester::testSlot);
    QObject::connect(t1, &QSignalTransition::triggered, &t1Tester, &SignalConnectionTester::testSlot);

    testTrigger.emitTrigger();

    QCoreApplication::processEvents();

    QTRY_VERIFY(s1Tester.testPassed);
    QTRY_VERIFY(s2Tester.testPassed);
    QTRY_VERIFY(t1Tester.testPassed);

}

void tst_QState::parallelStateAndInitialState()
{
    QStateMachine machine;

    { // setting an initial state on a parallel state:
        QState a(QState::ParallelStates, &machine);
        QState b(&a);
        QVERIFY(!a.initialState());
        const QString warning
            = QString::asprintf("QState::setInitialState: ignoring attempt to set initial state of parallel state group %p", &a);
        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
        a.setInitialState(&b); // should produce a warning and do nothing.
        QVERIFY(!a.initialState());
    }

    { // setting the child-mode from ExclusiveStates to ParallelStates should remove the initial state:
        QState a(QState::ExclusiveStates, &machine);
        QState b(&a);
        a.setInitialState(&b);
        QCOMPARE(a.initialState(), &b);
        const QString warning
            = QString::asprintf("QState::setChildMode: setting the child-mode of state %p to "
                                "parallel removes the initial state", &a);
        QTest::ignoreMessage(QtWarningMsg, qPrintable(warning));
        a.setChildMode(QState::ParallelStates); // should produce a warning and remove the initial state
        QVERIFY(!a.initialState());
        QCOMPARE(a.childMode(), QState::ParallelStates);
    }
}

QTEST_MAIN(tst_QState)
#include "tst_qstate.moc"
