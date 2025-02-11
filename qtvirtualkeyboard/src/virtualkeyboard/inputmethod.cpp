// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtVirtualKeyboard/private/inputmethod_p.h>
#include <QtVirtualKeyboard/qvirtualkeyboardtrace.h>
#include <QVariant>

QT_BEGIN_NAMESPACE
namespace QtVirtualKeyboard {

/*!
    \qmltype InputMethod
    \nativetype QtVirtualKeyboard::InputMethod
    \inqmlmodule QtQuick.VirtualKeyboard
    \ingroup qtvirtualkeyboard-internal-qml
    \brief Base type for creating input method in QML.

    The InputMethod type lets you create a custom input method
    which can be assigned to InputEngine.
*/

/*!
    \qmlproperty InputContext InputMethod::inputContext

    The input context.
*/

/*!
    \qmlproperty InputEngine InputMethod::inputEngine

    The input engine.
*/

/*!
    \qmlmethod list<int> InputMethod::inputModes(string locale)

    Returns a list of input modes for \a locale.
*/

/*!
    \qmlmethod bool InputMethod::setInputMode(string locale, int inputMode)

    Changes \a inputMode and \a locale for this input method. The method returns
    \c true if successful.
*/

/*!
    \qmlmethod bool InputMethod::setTextCase(int textCase)

    Updates \a textCase for this input method. The method returns \c true if
    successful.

    The possible values for the text case are:

    \list
        \li \c InputEngine.Lower Lower case text.
        \li \c InputEngine.Upper Upper case text.
    \endlist
*/

/*!
    \qmlmethod bool InputMethod::keyEvent(int key, string text, int modifiers)

    The purpose of this method is to handle the key events generated by the the
    input engine.

    The \a key parameter specifies the code of the key to handle. The key code
    does not distinguish between capital and non-capital letters. The \a
    text parameter contains the Unicode text for the key. The \a modifiers
    parameter contains the key modifiers that apply to \a key.

    This method returns \c true if the key event was successfully handled.
    If the return value is \c false, the key event is redirected to the default
    input method for further processing.
*/

/*!
    \qmlmethod InputMethod::reset()

    This method is called by the input engine when this input method needs to be
    reset. The input method must reset its internal state only. The main
    difference to the update() method is that reset() modifies only
    the input method state; it must not modify the input context.
*/

/*!
    \qmlmethod InputMethod::update()

    This method is called by the input engine when the input method needs to be
    updated. The input method must close the current pre-edit text and
    restore the internal state to the default.
*/

/*!
    \qmlmethod list<int> InputMethod::selectionLists()

    Returns the list of selection types used for this input method.

    This method is called by input engine when the input method is being
    activated and every time the input method hints are updated. The input method
    can reserve selection lists by returning the desired selection list types.

    The input method may request the input engine to update the selection lists
    at any time by emitting selectionListsChanged() signal. This signal will
    trigger a call to this method, allowing the input method to update the selection
    list types.
*/

/*!
    \qmlmethod int InputMethod::selectionListItemCount(int type)

    Returns the number of items in the selection list identified by \a type.
*/

/*!
    \qmlmethod var InputMethod::selectionListData(int type, int index, int role)

    Returns item data for a selection list identified by \a type. The \a role
    parameter specifies which data is requested. The \a index parameter is a
    zero based index into the selecteion list.
*/

/*!
    \qmlmethod void InputMethod::selectionListItemSelected(int type, int index)

    This method is called when an item at \a index has been selected by the
    user. The selection list is identified by the \a type parameter.
*/

/*!
    \qmlsignal InputMethod::selectionListChanged(int type)

    The input method emits this signal when the contents of the selection list
    are changed. The \a type parameter specifies which selection list has
    changed.
*/

/*!
    \qmlsignal InputMethod::selectionListActiveItemChanged(int type, int index)

    The input method emits this signal when the current \a index has changed
    in the selection list identified by \a type.
*/

/*!
    \qmlsignal InputMethod::selectionListsChanged()
    \since QtQuick.VirtualKeyboard 2.2

    The input method emits this signal when the selection list types have
    changed. This signal will trigger a call to selectionLists() method,
    allowing the input method to update the selection list types.
*/

/*!
    \qmlmethod list<int> InputMethod::patternRecognitionModes()
    \since QtQuick.VirtualKeyboard 2.0

    Returns list of supported pattern recognition modes.

    This method is invoked by the input engine to query the list of
    supported pattern recognition modes.
*/

/*!
    \qmlmethod Trace InputMethod::traceBegin(int traceId, int patternRecognitionMode, var traceCaptureDeviceInfo, var traceScreenInfo)
    \since QtQuick.VirtualKeyboard 2.0

    This method is called when a trace interaction starts with the specified \a patternRecognitionMode.
    The trace is uniquely identified by the \a traceId.
    The \a traceCaptureDeviceInfo provides information about the source device and the
    \a traceScreenInfo provides information about the screen context.

    If the input method accepts the event and wants to capture the trace input, it must return
    a new Trace object. This object must remain valid until the \l {InputMethod::traceEnd()}
    {InputMethod.traceEnd()} method is called. If the Trace is rendered on screen, it remains there
    until the Trace object is destroyed.
*/

/*!
    \qmlmethod bool InputMethod::traceEnd(Trace trace)
    \since QtQuick.VirtualKeyboard 2.0

    This method is called when the trace interaction ends. The input method should destroy the \a trace object
    at some point after this function is called. Returns \c true on success.

    See the \l {Trace API for Input Methods} how to access the gathered data.
*/

/*!
    \qmlmethod bool InputMethod::reselect(int cursorPosition, int reselectFlags)
    \since QtQuick.VirtualKeyboard 2.0

    This method attempts to reselect a word located at the \a cursorPosition.
    The \a reselectFlags define the rules for how the word should be selected in
    relation to the cursor position.

    \list
        \li \c InputEngine.WordBeforeCursor Activate the word before the cursor. When this flag is used exclusively, the word must end exactly at the cursor.
        \li \c InputEngine.WordAfterCursor Activate the word after the cursor. When this flag is used exclusively, the word must start exactly at the cursor.
        \li \c InputEngine.WordAtCursor Activate the word at the cursor. This flag is a combination of the above flags with the exception that the word cannot start or stop at the cursor.
    \endlist

    The method returns \c true if the word was successfully reselected.
*/

/*!
    \qmlmethod bool InputMethod::clickPreeditText(int cursorPosition)
    \since QtQuick.VirtualKeyboard 2.4

    Called when the user clicks on pre-edit text at \a cursorPosition.

    The function should return \c true if it handles the event. Otherwise the input
    falls back to \l reselect() for further processing.
*/

/*!
    \class QtVirtualKeyboard::InputMethod
    \internal
*/

InputMethod::InputMethod(QObject *parent) :
    QVirtualKeyboardAbstractInputMethod(parent)
{
}

InputMethod::~InputMethod()
{
}

QList<QVirtualKeyboardInputEngine::InputMode> InputMethod::inputModes(const QString &locale)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "inputModes",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, locale));
    QList<QVirtualKeyboardInputEngine::InputMode> inputModeList;
    const auto resultList = result.toList();
    inputModeList.reserve(resultList.size());
    for (const QVariant &inputMode : resultList)
        inputModeList.append(static_cast<QVirtualKeyboardInputEngine::InputMode>(inputMode.toInt()));
    return inputModeList;
}

bool InputMethod::setInputMode(const QString &locale, QVirtualKeyboardInputEngine::InputMode inputMode)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "setInputMode",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, locale),
                              Q_ARG(QVariant, static_cast<int>(inputMode)));
    return result.toBool();
}

bool InputMethod::setTextCase(QVirtualKeyboardInputEngine::TextCase textCase)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "setTextCase",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, static_cast<int>(textCase)));
    return result.toBool();
}

bool InputMethod::keyEvent(Qt::Key key, const QString &text, Qt::KeyboardModifiers modifiers)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "keyEvent",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, key),
                              Q_ARG(QVariant, text),
                              Q_ARG(QVariant, (int)modifiers));
    return result.toBool();
}

QList<QVirtualKeyboardSelectionListModel::Type> InputMethod::selectionLists()
{
    QVariant result;
    QMetaObject::invokeMethod(this, "selectionLists",
                              Q_RETURN_ARG(QVariant, result));
    QList<QVirtualKeyboardSelectionListModel::Type> selectionListsList;
    const auto resultList = result.toList();
    selectionListsList.reserve(resultList.size());
    for (const QVariant &selectionListType : resultList)
        selectionListsList.append(static_cast<QVirtualKeyboardSelectionListModel::Type>(selectionListType.toInt()));

    return selectionListsList;
}

int InputMethod::selectionListItemCount(QVirtualKeyboardSelectionListModel::Type type)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "selectionListItemCount",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, static_cast<int>(type)));
    return result.toInt();
}

QVariant InputMethod::selectionListData(QVirtualKeyboardSelectionListModel::Type type, int index, QVirtualKeyboardSelectionListModel::Role role)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "selectionListData",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, static_cast<int>(type)),
                              Q_ARG(QVariant, index),
                              Q_ARG(QVariant, static_cast<int>(role)));
    if (result.isNull()) {
        result = QVirtualKeyboardAbstractInputMethod::selectionListData(type, index, role);
    }
    return result;
}

void InputMethod::selectionListItemSelected(QVirtualKeyboardSelectionListModel::Type type, int index)
{
    QMetaObject::invokeMethod(this, "selectionListItemSelected",
                              Q_ARG(QVariant, static_cast<int>(type)),
                              Q_ARG(QVariant, index));
}

QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> InputMethod::patternRecognitionModes() const
{
    QVariant result;
    QMetaObject::invokeMethod(const_cast<InputMethod *>(this), "patternRecognitionModes",
                              Q_RETURN_ARG(QVariant, result));
    QList<QVirtualKeyboardInputEngine::PatternRecognitionMode> patterRecognitionModeList;
    const auto resultList = result.toList();
    patterRecognitionModeList.reserve(resultList.size());
    for (const QVariant &patterRecognitionMode : resultList)
        patterRecognitionModeList.append(static_cast<QVirtualKeyboardInputEngine::PatternRecognitionMode>(patterRecognitionMode.toInt()));

    return patterRecognitionModeList;
}

QVirtualKeyboardTrace *InputMethod::traceBegin(
        int traceId, QVirtualKeyboardInputEngine::PatternRecognitionMode patternRecognitionMode,
        const QVariantMap &traceCaptureDeviceInfo, const QVariantMap &traceScreenInfo)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "traceBegin",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(int, traceId),
                              Q_ARG(int, (int)patternRecognitionMode),
                              Q_ARG(QVariant, traceCaptureDeviceInfo),
                              Q_ARG(QVariant, traceScreenInfo));
    return result.value<QVirtualKeyboardTrace *>();
}

bool InputMethod::traceEnd(QVirtualKeyboardTrace *trace)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "traceEnd",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(QVariant, QVariant::fromValue(trace)));
    return result.toBool();
}

bool InputMethod::reselect(int cursorPosition, const QVirtualKeyboardInputEngine::ReselectFlags &reselectFlags)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "reselect",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(int, cursorPosition),
                              Q_ARG(int, (int)reselectFlags));
    return result.toBool();
}

bool InputMethod::clickPreeditText(int cursorPosition)
{
    QVariant result;
    QMetaObject::invokeMethod(this, "clickPreeditText",
                              Q_RETURN_ARG(QVariant, result),
                              Q_ARG(int, cursorPosition));
    return result.toBool();
}

void InputMethod::reset()
{
    QMetaObject::invokeMethod(this, "reset");
}

void InputMethod::update()
{
    QMetaObject::invokeMethod(this, "update");
}

void InputMethod::clearInputMode()
{
    QMetaObject::invokeMethod(this, "clearInputMode");
}

} // namespace QtVirtualKeyboard
QT_END_NAMESPACE
