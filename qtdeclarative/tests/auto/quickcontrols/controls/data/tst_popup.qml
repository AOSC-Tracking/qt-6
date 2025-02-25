// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Templates as T
import QtQuick.NativeStyle as NativeStyle
import Qt.test.controls

TestCase {
    id: testCase
    width: 400
    height: 400
    visible: true
    when: windowShown
    name: "Popup"

    ApplicationWindow {
        id: applicationWindow
        width: 480
        height: 360
    }

    Component {
        id: popupTemplate
        T.Popup { }
    }

    Component {
        id: popupControl
        Popup { }
    }

    Component {
        id: rect
        Rectangle { }
    }

    Component {
        id: signalSpy
        SignalSpy { }
    }

    function init() {
        failOnWarning(/.?/)
    }

    function test_defaults() {
        let control = createTemporaryObject(popupControl, testCase)
        verify(control)
    }

    function test_padding() {
        let control = createTemporaryObject(popupTemplate, testCase)
        verify(control)

        let paddingSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "paddingChanged"})
        verify(paddingSpy.valid)

        let topPaddingSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "topPaddingChanged"})
        verify(topPaddingSpy.valid)

        let leftPaddingSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "leftPaddingChanged"})
        verify(leftPaddingSpy.valid)

        let rightPaddingSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "rightPaddingChanged"})
        verify(rightPaddingSpy.valid)

        let bottomPaddingSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "bottomPaddingChanged"})
        verify(bottomPaddingSpy.valid)

        let paddingChanges = 0
        let topPaddingChanges = 0
        let leftPaddingChanges = 0
        let rightPaddingChanges = 0
        let bottomPaddingChanges = 0

        compare(control.padding, 0)
        compare(control.topPadding, 0)
        compare(control.leftPadding, 0)
        compare(control.rightPadding, 0)
        compare(control.bottomPadding, 0)
        compare(control.availableWidth, 0)
        compare(control.availableHeight, 0)

        control.width = 100
        control.height = 100

        control.padding = 10
        compare(control.padding, 10)
        compare(control.topPadding, 10)
        compare(control.leftPadding, 10)
        compare(control.rightPadding, 10)
        compare(control.bottomPadding, 10)
        compare(paddingSpy.count, ++paddingChanges)
        compare(topPaddingSpy.count, ++topPaddingChanges)
        compare(leftPaddingSpy.count, ++leftPaddingChanges)
        compare(rightPaddingSpy.count, ++rightPaddingChanges)
        compare(bottomPaddingSpy.count, ++bottomPaddingChanges)

        control.topPadding = 20
        compare(control.padding, 10)
        compare(control.topPadding, 20)
        compare(control.leftPadding, 10)
        compare(control.rightPadding, 10)
        compare(control.bottomPadding, 10)
        compare(paddingSpy.count, paddingChanges)
        compare(topPaddingSpy.count, ++topPaddingChanges)
        compare(leftPaddingSpy.count, leftPaddingChanges)
        compare(rightPaddingSpy.count, rightPaddingChanges)
        compare(bottomPaddingSpy.count, bottomPaddingChanges)

        control.leftPadding = 30
        compare(control.padding, 10)
        compare(control.topPadding, 20)
        compare(control.leftPadding, 30)
        compare(control.rightPadding, 10)
        compare(control.bottomPadding, 10)
        compare(paddingSpy.count, paddingChanges)
        compare(topPaddingSpy.count, topPaddingChanges)
        compare(leftPaddingSpy.count, ++leftPaddingChanges)
        compare(rightPaddingSpy.count, rightPaddingChanges)
        compare(bottomPaddingSpy.count, bottomPaddingChanges)

        control.rightPadding = 40
        compare(control.padding, 10)
        compare(control.topPadding, 20)
        compare(control.leftPadding, 30)
        compare(control.rightPadding, 40)
        compare(control.bottomPadding, 10)
        compare(paddingSpy.count, paddingChanges)
        compare(topPaddingSpy.count, topPaddingChanges)
        compare(leftPaddingSpy.count, leftPaddingChanges)
        compare(rightPaddingSpy.count, ++rightPaddingChanges)
        compare(bottomPaddingSpy.count, bottomPaddingChanges)

        control.bottomPadding = 50
        compare(control.padding, 10)
        compare(control.topPadding, 20)
        compare(control.leftPadding, 30)
        compare(control.rightPadding, 40)
        compare(control.bottomPadding, 50)
        compare(paddingSpy.count, paddingChanges)
        compare(topPaddingSpy.count, topPaddingChanges)
        compare(leftPaddingSpy.count, leftPaddingChanges)
        compare(rightPaddingSpy.count, rightPaddingChanges)
        compare(bottomPaddingSpy.count, ++bottomPaddingChanges)

        control.padding = 60
        compare(control.padding, 60)
        compare(control.topPadding, 20)
        compare(control.leftPadding, 30)
        compare(control.rightPadding, 40)
        compare(control.bottomPadding, 50)
        compare(paddingSpy.count, ++paddingChanges)
        compare(topPaddingSpy.count, topPaddingChanges)
        compare(leftPaddingSpy.count, leftPaddingChanges)
        compare(rightPaddingSpy.count, rightPaddingChanges)
        compare(bottomPaddingSpy.count, bottomPaddingChanges)
    }

    function test_availableSize() {
        let control = createTemporaryObject(popupTemplate, testCase)
        verify(control)

        let availableWidthSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "availableWidthChanged"})
        verify(availableWidthSpy.valid)

        let availableHeightSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "availableHeightChanged"})
        verify(availableHeightSpy.valid)

        let availableWidthChanges = 0
        let availableHeightChanges = 0

        control.width = 100
        compare(control.availableWidth, 100)
        compare(availableWidthSpy.count, ++availableWidthChanges)
        compare(availableHeightSpy.count, availableHeightChanges)

        control.height = 100
        compare(control.availableHeight, 100)
        compare(availableWidthSpy.count, availableWidthChanges)
        compare(availableHeightSpy.count, ++availableHeightChanges)

        control.padding = 10
        compare(control.availableWidth, 80)
        compare(control.availableHeight, 80)
        compare(availableWidthSpy.count, ++availableWidthChanges)
        compare(availableHeightSpy.count, ++availableHeightChanges)

        control.topPadding = 20
        compare(control.availableWidth, 80)
        compare(control.availableHeight, 70)
        compare(availableWidthSpy.count, availableWidthChanges)
        compare(availableHeightSpy.count, ++availableHeightChanges)

        control.leftPadding = 30
        compare(control.availableWidth, 60)
        compare(control.availableHeight, 70)
        compare(availableWidthSpy.count, ++availableWidthChanges)
        compare(availableHeightSpy.count, availableHeightChanges)

        control.rightPadding = 40
        compare(control.availableWidth, 30)
        compare(control.availableHeight, 70)
        compare(availableWidthSpy.count, ++availableWidthChanges)
        compare(availableHeightSpy.count, availableHeightChanges)

        control.bottomPadding = 50
        compare(control.availableWidth, 30)
        compare(control.availableHeight, 30)
        compare(availableWidthSpy.count, availableWidthChanges)
        compare(availableHeightSpy.count, ++availableHeightChanges)

        control.padding = 60
        compare(control.availableWidth, 30)
        compare(control.availableHeight, 30)
        compare(availableWidthSpy.count, availableWidthChanges)
        compare(availableHeightSpy.count, availableHeightChanges)

        control.width = 0
        compare(control.availableWidth, 0)
        compare(availableWidthSpy.count, ++availableWidthChanges)
        compare(availableHeightSpy.count, availableHeightChanges)

        control.height = 0
        compare(control.availableHeight, 0)
        compare(availableWidthSpy.count, availableWidthChanges)
        compare(availableHeightSpy.count, ++availableHeightChanges)
    }

    function test_position() {
        let control = createTemporaryObject(popupControl, testCase, {visible: true, leftMargin: 10, topMargin: 20, width: 100, height: 100})
        verify(control)
        if (control.popupType === Popup.Window)
            skip("Popup windows do not support margins and their position is not bound by their parent.")
        verify(control.visible)

        let xSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "xChanged"})
        verify(xSpy.valid)

        let ySpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "yChanged"})
        verify(ySpy.valid)

        // moving outside margins does not trigger change notifiers
        control.x = -100
        compare(control.x, 10)
        compare(control.y, 20)
        compare(xSpy.count, 0)
        compare(ySpy.count, 0)

        control.y = -200
        compare(control.x, 10)
        compare(control.y, 20)
        compare(xSpy.count, 0)
        compare(ySpy.count, 0)

        // moving within margins triggers change notifiers
        control.x = 30
        compare(control.x, 30)
        compare(control.y, 20)
        compare(xSpy.count, 1)
        compare(ySpy.count, 0)

        control.y = 40
        compare(control.x, 30)
        compare(control.y, 40)
        compare(xSpy.count, 1)
        compare(ySpy.count, 1)

        // re-parent and reset the position
        control.parent = createTemporaryObject(rect, testCase, {color: "red", width: 100, height: 100})
        control.x = 0
        control.y = 0
        compare(xSpy.count, 2)
        compare(ySpy.count, 2)

        // moving parent outside margins triggers change notifiers
        control.parent.x = -50
        compare(control.x, 50 + control.leftMargin)
        compare(xSpy.count, 3)
        compare(ySpy.count, 2)

        control.parent.y = -60
        compare(control.y, 60 + control.topMargin)
        compare(xSpy.count, 3)
        compare(ySpy.count, 3)
    }

    function test_resetSize() {
        let control = createTemporaryObject(popupControl, testCase, {visible: true, margins: 0})
        verify(control)

        control.popupType = Popup.Item
        control.scale = 1.0
        control.width = control.implicitWidth = testCase.width + 10
        control.height = control.implicitHeight = testCase.height + 10

        compare(control.width, testCase.width + 10)
        compare(control.height, testCase.height + 10)

        control.width = undefined
        control.height = undefined
        compare(control.width, testCase.width)
        compare(control.height, testCase.height)
    }

    function test_negativeMargins() {
        let control = createTemporaryObject(popupControl, testCase, {implicitWidth: testCase.width, implicitHeight: testCase.height})
        verify(control)
        if (control.popupType === Popup.Window)
            skip("Margins are not supported for Popup Window types.")

        control.open()
        verify(control.visible)

        compare(control.x, 0)
        compare(control.y, 0)

        compare(control.margins, -1)
        compare(control.topMargin, -1)
        compare(control.leftMargin, -1)
        compare(control.rightMargin, -1)
        compare(control.bottomMargin, -1)

        control.x = -10
        control.y = -10
        compare(control.x, -10)
        compare(control.y, -10)
    }

    function test_margins() {
        let control = createTemporaryObject(popupTemplate, testCase, {width: 100, height: 100})
        verify(control)
        if (control.popupType === Popup.Window)
            skip("Margins are not supported for Popup Window types.")

        control.open()
        verify(control.visible)

        control.margins = 10
        compare(control.margins, 10)
        compare(control.topMargin, 10)
        compare(control.leftMargin, 10)
        compare(control.rightMargin, 10)
        compare(control.bottomMargin, 10)
        compare(control.contentItem.parent.x, 10)
        compare(control.contentItem.parent.y, 10)

        control.topMargin = 20
        compare(control.margins, 10)
        compare(control.topMargin, 20)
        compare(control.leftMargin, 10)
        compare(control.rightMargin, 10)
        compare(control.bottomMargin, 10)
        compare(control.contentItem.parent.x, 10)
        compare(control.contentItem.parent.y, 20)

        control.leftMargin = 20
        compare(control.margins, 10)
        compare(control.topMargin, 20)
        compare(control.leftMargin, 20)
        compare(control.rightMargin, 10)
        compare(control.bottomMargin, 10)
        compare(control.contentItem.parent.x, 20)
        compare(control.contentItem.parent.y, 20)

        control.x = testCase.width
        control.y = testCase.height
        compare(control.contentItem.parent.x, testCase.width - control.width - 10)
        compare(control.contentItem.parent.y, testCase.height - control.height - 10)

        control.rightMargin = 20
        compare(control.margins, 10)
        compare(control.topMargin, 20)
        compare(control.leftMargin, 20)
        compare(control.rightMargin, 20)
        compare(control.bottomMargin, 10)
        compare(control.contentItem.parent.x, testCase.width - control.width - 20)
        compare(control.contentItem.parent.y, testCase.height - control.height - 10)

        control.bottomMargin = 20
        compare(control.margins, 10)
        compare(control.topMargin, 20)
        compare(control.leftMargin, 20)
        compare(control.rightMargin, 20)
        compare(control.bottomMargin, 20)
        compare(control.contentItem.parent.x, testCase.width - control.width - 20)
        compare(control.contentItem.parent.y, testCase.height - control.height - 20)

        control.margins = undefined
        compare(control.margins, -1)

        control.bottomMargin = undefined
        compare(control.bottomMargin, -1)
        compare(control.contentItem.parent.x, testCase.width - control.width - 20)
        compare(control.contentItem.parent.y, testCase.height)

        control.rightMargin = undefined
        compare(control.rightMargin, -1)
        compare(control.contentItem.parent.x, testCase.width)
        compare(control.contentItem.parent.y, testCase.height)

        control.x = -testCase.width
        control.y = -testCase.height
        compare(control.contentItem.parent.x, 20)
        compare(control.contentItem.parent.y, 20)

        control.topMargin = undefined
        compare(control.topMargin, -1)
        compare(control.contentItem.parent.x, 20)
        compare(control.contentItem.parent.y, -testCase.height)

        control.leftMargin = undefined
        compare(control.leftMargin, -1)
        compare(control.contentItem.parent.x, -testCase.width)
        compare(control.contentItem.parent.y, -testCase.height)
    }

    function test_background() {
        let control = createTemporaryObject(popupTemplate, testCase)
        verify(control)

        control.background = rect.createObject(testCase)

        // background has no x or width set, so its width follows control's width
        control.width = 320
        compare(control.background.width, control.width)

        // background has no y or height set, so its height follows control's height
        compare(control.background.height, control.height)
        control.height = 240

        // has width => width does not follow
        control.background.width /= 2
        control.width += 20
        verify(control.background.width !== control.width)

        // reset width => width follows again
        control.background.width = undefined
        control.width += 20
        compare(control.background.width, control.width)

        // has x => width does not follow
        control.background.x = 10
        control.width += 20
        verify(control.background.width !== control.width)

        // has height => height does not follow
        control.background.height /= 2
        control.height -= 20
        verify(control.background.height !== control.height)

        // reset height => height follows again
        control.background.height = undefined
        control.height -= 20
        compare(control.background.height, control.height)

        // has y => height does not follow
        control.background.y = 10
        control.height -= 20
        verify(control.background.height !== control.height)
    }

    function getChild(control, objname, idx) {
        let index = idx
        for (let i = index+1; i < control.children.length; i++)
        {
            if (control.children[i].objectName === objname) {
                index = i
                break
            }
        }
        return index
    }

    Component {
        id: component
        ApplicationWindow {
            id: _window
            width: 400
            height: 400
            visible: true
            font.pixelSize: 40
            property alias pane: _pane
            property alias popup: _popup
            property SignalSpy fontspy: SignalSpy { target: _window; signalName: "fontChanged" }
            Pane {
                id: _pane
                property alias button: _button
                font.pixelSize: 30
                property SignalSpy fontspy: SignalSpy { target: _pane; signalName: "fontChanged" }
                Column {
                    Button {
                        id: _button
                        text: "Button"
                        font.pixelSize: 20
                        property SignalSpy fontspy: SignalSpy { target: _button; signalName: "fontChanged" }
                        Popup {
                            id: _popup
                            property alias button: _button2
                            property alias listview: _listview
                            y: _button.height
                            implicitHeight: Math.min(396, _listview.contentHeight)
                            property SignalSpy fontspy: SignalSpy { target: _popup; signalName: "fontChanged" }
                            contentItem: Column {
                                Button {
                                    id: _button2
                                    text: "Button"
                                    property SignalSpy fontspy: SignalSpy { target: _button2; signalName: "fontChanged" }
                                }
                                ListView {
                                    id: _listview
                                    height: _button.height * 20
                                    model: 2
                                    delegate: Button {
                                        id: _button3
                                        objectName: "delegate"
                                        width: _button.width
                                        height: _button.height
                                        text: "N: " + index
                                        checkable: true
                                        autoExclusive: true
                                        property SignalSpy fontspy: SignalSpy { target: _button3; signalName: "fontChanged" }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function test_font() { // QTBUG_50984, QTBUG-51696
        let window = createTemporaryObject(component, testCase)
        verify(window)

        compare(window.font.pixelSize, 40)
        compare(window.pane.font.pixelSize, 30)
        compare(window.pane.button.font.pixelSize, 20)
        compare(window.popup.font.pixelSize, 40)
        let idx1 = getChild(window.popup.listview.contentItem, "delegate", -1)
        let idx2 = getChild(window.popup.listview.contentItem, "delegate", idx1)
        window.popup.listview.contentItem.children[idx1].fontspy.clear()
        window.popup.listview.contentItem.children[idx2].fontspy.clear()
        window.popup.button.fontspy.clear()
        compare(window.popup.button.font.pixelSize, 40)
        compare(window.popup.listview.contentItem.children[idx1].font.pixelSize, 40)
        compare(window.popup.listview.contentItem.children[idx2].font.pixelSize, 40)

        window.pane.button.font.pixelSize = 30
        compare(window.font.pixelSize, 40)
        compare(window.fontspy.count, 0)
        compare(window.pane.font.pixelSize, 30)
        compare(window.pane.fontspy.count, 0)
        compare(window.pane.button.font.pixelSize, 30)
        compare(window.pane.button.fontspy.count, 1)
        compare(window.popup.font.pixelSize, 40)
        compare(window.popup.fontspy.count, 0)
        compare(window.popup.button.fontspy.count, 0)
        compare(window.popup.button.font.pixelSize, 40)
        compare(window.popup.listview.contentItem.children[idx1].font.pixelSize, 40)
        compare(window.popup.listview.contentItem.children[idx2].font.pixelSize, 40)
        compare(window.popup.listview.contentItem.children[idx1].fontspy.count, 0)
        compare(window.popup.listview.contentItem.children[idx2].fontspy.count, 0)


        window.font.pixelSize = 50
        compare(window.font.pixelSize, 50)
        compare(window.fontspy.count, 1)
        compare(window.pane.font.pixelSize, 30)
        compare(window.pane.fontspy.count, 0)
        compare(window.pane.button.font.pixelSize, 30)
        compare(window.pane.button.fontspy.count, 1)
        compare(window.popup.font.pixelSize, 50)
        compare(window.popup.fontspy.count, 1)
        compare(window.popup.button.font.pixelSize, 50)
        compare(window.popup.button.fontspy.count, 1)
        compare(window.popup.listview.contentItem.children[idx1].font.pixelSize, 50)
        compare(window.popup.listview.contentItem.children[idx1].fontspy.count, 1)
        compare(window.popup.listview.contentItem.children[idx2].font.pixelSize, 50)
        compare(window.popup.listview.contentItem.children[idx2].fontspy.count, 1)


        window.popup.button.font.pixelSize = 10
        compare(window.font.pixelSize, 50)
        compare(window.fontspy.count, 1)
        compare(window.pane.font.pixelSize, 30)
        compare(window.pane.fontspy.count, 0)
        compare(window.pane.button.font.pixelSize, 30)
        compare(window.pane.button.fontspy.count, 1)
        compare(window.popup.font.pixelSize, 50)
        compare(window.popup.fontspy.count, 1)
        compare(window.popup.button.font.pixelSize, 10)
        compare(window.popup.button.fontspy.count, 2)
        compare(window.popup.listview.contentItem.children[idx1].font.pixelSize, 50)
        compare(window.popup.listview.contentItem.children[idx1].fontspy.count, 1)
        compare(window.popup.listview.contentItem.children[idx2].font.pixelSize, 50)
        compare(window.popup.listview.contentItem.children[idx2].fontspy.count, 1)


        window.popup.font.pixelSize = 60
        compare(window.font.pixelSize, 50)
        compare(window.fontspy.count, 1)
        compare(window.pane.font.pixelSize, 30)
        compare(window.pane.fontspy.count, 0)
        compare(window.pane.button.font.pixelSize, 30)
        compare(window.pane.button.fontspy.count, 1)
        compare(window.popup.font.pixelSize, 60)
        compare(window.popup.fontspy.count, 2)
        compare(window.popup.button.font.pixelSize, 10)
        compare(window.popup.button.fontspy.count, 2)
        compare(window.popup.listview.contentItem.children[idx1].font.pixelSize, 60)
        compare(window.popup.listview.contentItem.children[idx1].fontspy.count, 2)
        compare(window.popup.listview.contentItem.children[idx2].font.pixelSize, 60)
        compare(window.popup.listview.contentItem.children[idx2].fontspy.count, 2)
    }

    Component {
        id: localeComponent
        Pane {
            property alias button: _button
            property alias popup: _popup
            locale: Qt.locale("en_US")
            Column {
                Button {
                    id: _button
                    text: "Button"
                    locale: Qt.locale("nb_NO")
                    Popup {
                        id: _popup
                        property alias button1: _button1
                        property alias button2: _button2
                        y: _button.height
                        locale: Qt.locale("fi_FI")
                        implicitHeight: Math.min(396, _column.contentHeight)
                        contentItem: Column {
                            id: _column
                            Button {
                                id: _button1
                                text: "Button 1"
                                objectName: "1"
                            }
                            Button {
                                id: _button2
                                text: "Button 2"
                                locale: Qt.locale("nb_NO")
                                objectName: "2"
                            }
                        }
                    }
                }
            }
        }
    }

    function test_locale() { // QTBUG_50984
        // test looking up natural locale from ancestors
        let control = createTemporaryObject(localeComponent, applicationWindow.contentItem)
        verify(control)

        compare(control.locale.name, "en_US")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "fi_FI")
        compare(control.popup.button1.locale.name, "fi_FI")
        compare(control.popup.button2.locale.name, "nb_NO")

        control.ApplicationWindow.window.locale = undefined
    }

    Component {
        id: localeChangeComponent
        Pane {
            id: _pane
            property alias button: _button
            property alias popup: _popup
            property SignalSpy localespy: SignalSpy {
                target: _pane
                signalName: "localeChanged"
            }
            property SignalSpy mirrorspy: SignalSpy {
                target: _pane
                signalName: "mirroredChanged"
            }
            Column {
                Button {
                    id: _button
                    text: "Button"
                    property SignalSpy localespy: SignalSpy {
                        target: _button
                        signalName: "localeChanged"
                    }
                    property SignalSpy mirrorspy: SignalSpy {
                        target: _button
                        signalName: "mirroredChanged"
                    }
                    Popup {
                        id: _popup
                        property alias button1: _button1
                        property alias button2: _button2
                        y: _button.height
                        implicitHeight: Math.min(396, _column.contentHeight)
                        property SignalSpy localespy: SignalSpy {
                            target: _popup
                            signalName: "localeChanged"
                        }
                        contentItem: Column {
                            id: _column
                            Button {
                                id: _button1
                                text: "Button 1"
                                property SignalSpy localespy: SignalSpy {
                                    target: _button1
                                    signalName: "localeChanged"
                                }
                                property SignalSpy mirrorspy: SignalSpy {
                                    target: _button1
                                    signalName: "mirroredChanged"
                                }
                            }
                            Button {
                                id: _button2
                                text: "Button 2"
                                property SignalSpy localespy: SignalSpy {
                                    target: _button2
                                    signalName: "localeChanged"
                                }
                                property SignalSpy mirrorspy: SignalSpy {
                                    target: _button2
                                    signalName: "mirroredChanged"
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    function test_locale_changes() { // QTBUG_50984
        // test default locale and locale inheritance
        let control = createTemporaryObject(localeChangeComponent, applicationWindow.contentItem)
        verify(control)

        let defaultLocale = Qt.locale()
        compare(control.ApplicationWindow.window.locale.name, defaultLocale.name)
        compare(control.locale.name, defaultLocale.name)
        compare(control.button.locale.name, defaultLocale.name)
        compare(control.popup.locale.name, defaultLocale.name)
        compare(control.popup.button1.locale.name, defaultLocale.name)
        compare(control.popup.button2.locale.name, defaultLocale.name)

        control.ApplicationWindow.window.locale = Qt.locale("nb_NO")
        compare(control.ApplicationWindow.window.locale.name, "nb_NO")
        compare(control.locale.name, "nb_NO")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "nb_NO")
        compare(control.popup.button1.locale.name, "nb_NO")
        compare(control.popup.button2.locale.name, "nb_NO")
        compare(control.localespy.count, 1)
        compare(control.button.localespy.count, 1)
        compare(control.popup.localespy.count, 1)
        compare(control.popup.button1.localespy.count, 1)
        compare(control.popup.button2.localespy.count, 1)

        control.ApplicationWindow.window.locale = undefined
        compare(control.ApplicationWindow.window.locale.name, defaultLocale.name)
        compare(control.locale.name, defaultLocale.name)
        compare(control.button.locale.name, defaultLocale.name)
        compare(control.popup.locale.name, defaultLocale.name)
        compare(control.popup.button1.locale.name, defaultLocale.name)
        compare(control.popup.button2.locale.name, defaultLocale.name)
        compare(control.localespy.count, 2)
        compare(control.button.localespy.count, 2)
        compare(control.popup.localespy.count, 2)
        compare(control.popup.button1.localespy.count, 2)
        compare(control.popup.button2.localespy.count, 2)

        control.locale = Qt.locale("ar_EG")
        compare(control.ApplicationWindow.window.locale.name, defaultLocale.name)
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "ar_EG")
        compare(control.popup.locale.name, defaultLocale.name)
        compare(control.popup.button1.locale.name, defaultLocale.name)
        compare(control.popup.button2.locale.name, defaultLocale.name)
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 3)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 2)
        compare(control.popup.button1.localespy.count, 2)
        compare(control.popup.button2.localespy.count, 2)

        control.ApplicationWindow.window.locale = Qt.locale("ar_EG")
        compare(control.ApplicationWindow.window.locale.name, "ar_EG")
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "ar_EG")
        compare(control.popup.locale.name, "ar_EG")
        compare(control.popup.button1.locale.name, "ar_EG")
        compare(control.popup.button2.locale.name, "ar_EG")
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 3)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 3)
        compare(control.popup.button1.localespy.count, 3)
        compare(control.popup.button1.mirrorspy.count, 0)
        compare(control.popup.button2.localespy.count, 3)
        compare(control.popup.button2.mirrorspy.count, 0)

        control.button.locale = Qt.locale("nb_NO")
        compare(control.ApplicationWindow.window.locale.name, "ar_EG")
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "ar_EG")
        compare(control.popup.button1.locale.name, "ar_EG")
        compare(control.popup.button2.locale.name, "ar_EG")
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 3)
        compare(control.popup.button1.localespy.count, 3)
        compare(control.popup.button2.localespy.count, 3)

        control.locale = undefined
        compare(control.ApplicationWindow.window.locale.name, "ar_EG")
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "ar_EG")
        compare(control.popup.button1.locale.name, "ar_EG")
        compare(control.popup.button2.locale.name, "ar_EG")
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 3)
        compare(control.popup.button1.localespy.count, 3)
        compare(control.popup.button2.localespy.count, 3)

        control.popup.button1.locale = Qt.locale("nb_NO")
        compare(control.ApplicationWindow.window.locale.name, "ar_EG")
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "ar_EG")
        compare(control.popup.button1.locale.name, "nb_NO")
        compare(control.popup.button2.locale.name, "ar_EG")
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 3)
        compare(control.popup.button1.localespy.count, 4)
        compare(control.popup.button1.mirrorspy.count, 0)
        compare(control.popup.button2.localespy.count, 3)
        compare(control.popup.button2.mirrorspy.count, 0)

        control.popup.locale = Qt.locale("fi_FI")
        compare(control.ApplicationWindow.window.locale.name, "ar_EG")
        compare(control.locale.name, "ar_EG")
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "fi_FI")
        compare(control.popup.button1.locale.name, "nb_NO")
        compare(control.popup.button2.locale.name, "fi_FI")
        compare(control.localespy.count, 3)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 4)
        compare(control.popup.button1.localespy.count, 4)
        compare(control.popup.button1.mirrorspy.count, 0)
        compare(control.popup.button2.localespy.count, 4)
        compare(control.popup.button2.mirrorspy.count, 0)

        control.ApplicationWindow.window.locale = undefined
        compare(control.ApplicationWindow.window.locale.name, defaultLocale.name)
        compare(control.locale.name, defaultLocale.name)
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, "fi_FI")
        compare(control.popup.button1.locale.name, "nb_NO")
        compare(control.popup.button2.locale.name, "fi_FI")
        compare(control.localespy.count, 4)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 4)
        compare(control.popup.button1.localespy.count, 4)
        compare(control.popup.button1.mirrorspy.count, 0)
        compare(control.popup.button2.localespy.count, 4)
        compare(control.popup.button2.mirrorspy.count, 0)

        control.popup.locale = undefined
        compare(control.ApplicationWindow.window.locale.name, defaultLocale.name)
        compare(control.locale.name, defaultLocale.name)
        compare(control.button.locale.name, "nb_NO")
        compare(control.popup.locale.name, defaultLocale.name)
        compare(control.popup.button1.locale.name, "nb_NO")
        compare(control.popup.button2.locale.name, defaultLocale.name)
        compare(control.localespy.count, 4)
        compare(control.mirrorspy.count, 0)
        compare(control.button.localespy.count, 4)
        compare(control.button.mirrorspy.count, 0)
        compare(control.popup.localespy.count, 5)
        compare(control.popup.button1.localespy.count, 4)
        compare(control.popup.button1.mirrorspy.count, 0)
        compare(control.popup.button2.localespy.count, 5)
        compare(control.popup.button2.mirrorspy.count, 0)
    }

    function test_size() {
        let control = createTemporaryObject(popupControl, testCase)
        verify(control)

        let openedSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "opened"})
        verify(openedSpy.valid)

        control.open()
        openedSpy.wait()
        compare(openedSpy.count, 1)
        verify(control.visible)

        // remove the background so that it won't affect the implicit size of the popup,
        // so the implicit sizes tested below are entirely based on the content size
        control.background = null

        // implicit size of the content
        control.contentItem.implicitWidth = 10
        compare(control.implicitWidth, 10 + control.leftPadding + control.rightPadding)
        compare(control.width, control.implicitWidth)
        compare(control.contentItem.width, control.availableWidth)

        control.contentItem.implicitHeight = 20
        compare(control.implicitHeight, 20 + control.topPadding + control.bottomPadding)
        compare(control.height, control.implicitHeight)
        compare(control.contentItem.height, control.availableHeight)

        // implicit size of the popup
        control.implicitWidth = 30
        compare(control.implicitWidth, 30)
        compare(control.width, 30)
        compare(control.contentItem.width, control.availableWidth)

        control.implicitHeight = 40
        compare(control.implicitHeight, 40)
        compare(control.height, 40)
        compare(control.contentItem.height, control.availableHeight)

        // set explicit size
        control.width = 50
        compare(control.implicitWidth, 30)
        tryCompare(control, "width", 50)
        compare(control.contentItem.width, control.availableWidth)

        control.height = 60
        compare(control.implicitHeight, 40)
        tryCompare(control, "height", 60)
        compare(control.contentItem.height, control.availableHeight)

        // reset explicit size
        control.width = undefined
        compare(control.implicitWidth, 30)
        compare(control.width, 30)
        compare(control.contentItem.width, control.availableWidth)

        control.height = undefined
        compare(control.implicitHeight, 40)
        compare(control.height, 40)
        compare(control.contentItem.height, control.availableHeight)
    }

    function test_visible() {
        let control = createTemporaryObject(popupTemplate, testCase, {visible: true})
        verify(control)

        // QTBUG-51989
        tryCompare(control, "visible", true)

        // QTBUG-55347
        control.parent = null
        verify(!control.visible)
    }

    Component {
        id: overlayTest
        ApplicationWindow {
            property alias firstDrawer: firstDrawer
            property alias secondDrawer: secondDrawer
            property alias modalPopup: modalPopup
            property alias modelessPopup: modelessPopup
            property alias plainPopup: plainPopup
            property alias modalPopupWithoutDim: modalPopupWithoutDim
            visible: true
            Drawer {
                id: firstDrawer
                z: 0
                popupType: Popup.Item
            }
            Drawer {
                id: secondDrawer
                z: 1
                popupType: Popup.Item
            }
            Popup {
                id: modalPopup
                z: 2
                popupType: Popup.Item
                modal: true
                exit: Transition { PauseAnimation { duration: 200 } }
            }
            Popup {
                id: modelessPopup
                z: 3
                popupType: Popup.Item
                dim: true
                exit: Transition { PauseAnimation { duration: 200 } }
            }
            Popup {
                id: plainPopup
                z: 4
                popupType: Popup.Item
                enter: Transition { PauseAnimation { duration: 200 } }
                exit: Transition { PauseAnimation { duration: 200 } }
            }
            Popup {
                id: modalPopupWithoutDim
                z: 5
                popupType: Popup.Item
                dim: false
                modal: true
                exit: Transition { PauseAnimation { duration: 200 } }
            }
        }
    }

    function indexOf(array, item) {
        for (let idx = 0; idx < array.length; ++idx) {
            if (item === array[idx])
                return idx;
        }
        return -1
    }

    function findOverlay(window, popup) {
        let item = popup.contentItem.parent
        let idx = indexOf(window.Overlay.overlay.children, item)
        return window.Overlay.overlay.children[idx - 1]
    }

    function test_overlay() {
        let window = createTemporaryObject(overlayTest, testCase)
        verify(window)

        window.requestActivate()
        tryCompare(window, "active", true)

        compare(window.Overlay.overlay.children.length, 0)

        let firstOverlay = findOverlay(window, window.firstDrawer)
        verify(!firstOverlay)
        window.firstDrawer.open()
        compare(window.Overlay.overlay.children.length, 2) // 1 drawer + 1 overlay
        firstOverlay = findOverlay(window, window.firstDrawer)
        verify(firstOverlay)
        compare(firstOverlay.z, window.firstDrawer.z)
        compare(indexOf(window.Overlay.overlay.children, firstOverlay),
                indexOf(window.Overlay.overlay.children, window.firstDrawer.contentItem.parent) - 1)
        tryCompare(firstOverlay, "opacity", 1.0)

        let secondOverlay = findOverlay(window, window.secondDrawer)
        verify(!secondOverlay)
        window.secondDrawer.open()
        compare(window.Overlay.overlay.children.length, 4) // 2 drawers + 2 overlays
        secondOverlay = findOverlay(window, window.secondDrawer)
        verify(secondOverlay)
        compare(secondOverlay.z, window.secondDrawer.z)
        compare(indexOf(window.Overlay.overlay.children, secondOverlay),
                indexOf(window.Overlay.overlay.children, window.secondDrawer.contentItem.parent) - 1)
        tryCompare(secondOverlay, "opacity", 1.0)

        window.firstDrawer.close()
        tryCompare(window.firstDrawer, "visible", false)
        firstOverlay = findOverlay(window, window.firstDrawer)
        verify(!firstOverlay)
        compare(window.Overlay.overlay.children.length, 2) // 1 drawer + 1 overlay

        window.secondDrawer.close()
        tryCompare(window.secondDrawer, "visible", false)
        secondOverlay = findOverlay(window, window.secondDrawer)
        verify(!secondOverlay)
        compare(window.Overlay.overlay.children.length, 0)

        let modalOverlay = findOverlay(window, window.modalPopup)
        verify(!modalOverlay)
        window.modalPopup.open()
        modalOverlay = findOverlay(window, window.modalPopup)
        verify(modalOverlay)
        compare(modalOverlay.z, window.modalPopup.z)
        compare(window.modalPopup.visible, true)
        tryCompare(modalOverlay, "opacity", 1.0)
        compare(window.Overlay.overlay.children.length, 2) // 1 popup + 1 overlay

        let modelessOverlay = findOverlay(window, window.modelessPopup)
        verify(!modelessOverlay)
        window.modelessPopup.open()
        modelessOverlay = findOverlay(window, window.modelessPopup)
        verify(modelessOverlay)
        compare(modelessOverlay.z, window.modelessPopup.z)
        compare(window.modelessPopup.visible, true)
        tryCompare(modelessOverlay, "opacity", 1.0)
        compare(window.Overlay.overlay.children.length, 4) // 2 popups + 2 overlays

        window.modelessPopup.close()
        tryCompare(window.modelessPopup, "visible", false)
        modelessOverlay = findOverlay(window, window.modelessPopup)
        verify(!modelessOverlay)
        compare(window.Overlay.overlay.children.length, 2) // 1 popup + 1 overlay

        compare(window.modalPopup.visible, true)
        compare(modalOverlay.opacity, 1.0)

        window.modalPopup.close()
        tryCompare(window.modalPopup, "visible", false)
        modalOverlay = findOverlay(window, window.modalPopup)
        verify(!modalOverlay)
        compare(window.Overlay.overlay.children.length, 0)

        window.plainPopup.open()
        tryCompare(window.plainPopup, "visible", true)
        compare(window.Overlay.overlay.children.length, 1) // only popup added, no overlays involved

        window.plainPopup.modal = true
        compare(window.Overlay.overlay.children.length, 2) // overlay added

        window.plainPopup.close()
        tryCompare(window.plainPopup, "visible", false)
        compare(window.Overlay.overlay.children.length, 0) // popup + overlay removed

        window.modalPopupWithoutDim.open()
        tryCompare(window.modalPopupWithoutDim, "visible", true)
        compare(window.Overlay.overlay.children.length, 1) // only popup added, no overlays involved

        window.modalPopupWithoutDim.dim = true
        compare(window.Overlay.overlay.children.length, 2) // overlay added

        window.modalPopupWithoutDim.close()
        tryCompare(window.modalPopupWithoutDim, "visible", false)
        compare(window.Overlay.overlay.children.length, 0) // popup + overlay removed
    }

    function test_attached_applicationwindow() {
        let control = createTemporaryObject(popupControl, applicationWindow.contentItem)
        verify(control)

        let child = rect.createObject(control.contentItem)

        compare(control.ApplicationWindow.window, applicationWindow)
        compare(control.contentItem.ApplicationWindow.window, applicationWindow)
        compare(child.ApplicationWindow.window, applicationWindow)

        control.parent = null
        compare(control.ApplicationWindow.window, null)
        compare(control.contentItem.ApplicationWindow.window, null)
        compare(child.ApplicationWindow.window, null)
    }

    Component {
        id: pausePopup
        Popup {
            enter: Transition { PauseAnimation { duration: 200 } }
            exit: Transition { PauseAnimation { duration: 200 } }
        }
    }

    function test_openedClosed() {
        let control = createTemporaryObject(pausePopup, testCase)
        verify(control)

        let openedSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "opened"})
        verify(openedSpy.valid)
        let closedSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "closed"})
        verify(closedSpy.valid)
        let openedChangeSpy = createTemporaryObject(signalSpy, testCase, {target: control, signalName: "openedChanged"})
        verify(openedChangeSpy.valid)

        control.open()
        compare(control.visible, true)
        compare(control.opened, false)
        compare(openedChangeSpy.count, 0)
        compare(openedSpy.count, 0)
        tryCompare(openedSpy, "count", 1)
        compare(control.opened, true)
        compare(openedChangeSpy.count, 1)
        compare(closedSpy.count, 0)

        control.close()
        compare(control.visible, true)
        compare(control.opened, false)
        compare(openedChangeSpy.count, 2)
        compare(openedSpy.count, 1)
        compare(closedSpy.count, 0)
        tryCompare(closedSpy, "count", 1)
        compare(control.opened, false)
        compare(openedChangeSpy.count, 2)
        compare(control.visible, false)
    }

    Component {
        id: xyBindingLoop
        ApplicationWindow {
            id: window
            width: 360
            height: 360
            visible: true
            property alias popup: popup

            Popup {
                id: popup
                visible: true
                x: (parent.width - width) / 2
                y: (parent.height - height) / 2
                Label {
                    text: "Content"
                    anchors.fill: parent
                }
            }
        }
    }

    function test_xyBindingLoop() {
        let window = createTemporaryObject(xyBindingLoop, testCase)
        let control = window.popup
        waitForRendering(control.contentItem)

        if (control.popupType === Popup.Item) {
            compare(control.x, (control.parent.width - control.width) / 2)
            compare(control.y, (control.parent.height - control.height) / 2)
        } else {
            compare(control.x, Math.floor((control.parent.width - control.width) / 2))
            compare(control.y, Math.floor((control.parent.height - control.height) / 2))
        }
    }

    function test_windowParent() {
        let control = createTemporaryObject(popupControl, applicationWindow, {width: 100, height: 100})
        verify(control)

        control.open()
        verify(control.visible)
    }

    function test_deferredBackgroundSize() {
        let control = createTemporaryObject(popupControl, testCase, {width: 200, height: 100})
        verify(control)

        compare(control.background.width, 200 + (control.background.leftInset || 0) + (control.background.rightInset || 0))
        compare(control.background.height, 100 + (control.background.topInset || 0) + (control.background.bottomInset || 0))
    }

    function test_anchors() {
        let control = createTemporaryObject(popupControl, applicationWindow.contentItem.Overlay.overlay,
            { visible: true, width: 100, height: 100 })

        applicationWindow.visible = true
        verify(waitForRendering(applicationWindow.contentItem))

        verify(control)
        verify(control.visible)
        // If there is a transition then make sure it is finished
        if (control.enter !== null)
            tryCompare(control.enter, "running", false)
        compare(control.parent, control.Overlay.overlay)
        compare(control.x, 0)
        compare(control.y, 0)

        let overlay = control.Overlay.overlay
        verify(overlay)

        let centerInSpy = createTemporaryObject(signalSpy, testCase, { target: control.anchors, signalName: "centerInChanged" })
        verify(centerInSpy.valid)

        verify(overlay.width > 0)
        verify(overlay.height > 0)

        const getCurrentPos = () => {
            const parentPos = control.parent.mapToGlobal(0, 0)
            const popupPos = control.contentItem.parent.mapToGlobal(0, 0)
            return Qt.point(popupPos.x - parentPos.x, popupPos.y - parentPos.y)
        }

        // Center the popup in the window via the overlay.
        control.anchors.centerIn = Qt.binding(function() { return control.parent; })
        compare(centerInSpy.count, 1)
        let currentPos = getCurrentPos()
        compare(currentPos.x, (overlay.width - (control.width * control.scale)) / 2)
        compare(currentPos.y, (overlay.height - (control.width * control.scale)) / 2)

        // Ensure that it warns when trying to set it to an item that's not its parent.
        let anotherItem = createTemporaryObject(rect, applicationWindow.contentItem, { x: 100, y: 100, width: 50, height: 50 })
        verify(anotherItem)

        ignoreWarning(new RegExp(".*QML Popup: Popup can only be centered within its immediate parent or Overlay.overlay"))
        control.anchors.centerIn = anotherItem
        // The property will change, because we can't be sure that the parent
        // in QQuickPopupAnchors::setCenterIn() is the final parent, as some reparenting can happen.
        // We still expect the warning from QQuickPopupPositioner::reposition() though.
        compare(centerInSpy.count, 2)
        compare(control.anchors.centerIn, anotherItem)

        // The binding to the popup's parent was broken above, so restore it.
        control.anchors.centerIn = Qt.binding(function() { return control.parent; })
        compare(centerInSpy.count, 3)

        // Change the popup's parent and ensure that it's anchored accordingly.
        control.parent = Qt.binding(function() { return anotherItem; })
        compare(control.parent, anotherItem)
        compare(control.anchors.centerIn, anotherItem)
        compare(centerInSpy.count, 4)
        currentPos = getCurrentPos()
        compare(currentPos.x, (anotherItem.width - (control.width * control.scale)) / 2)
        compare(currentPos.y, (anotherItem.height - (control.height * control.scale)) / 2)

        // Check that anchors.centerIn beats x and y coordinates as it does in QQuickItem.
        control.x = 33;
        control.y = 44;
        currentPos = getCurrentPos()
        compare(currentPos.x, (anotherItem.width - (control.width * control.scale)) / 2)
        compare(currentPos.y, (anotherItem.height - (control.height * control.scale)) / 2)

        // Check that the popup's x and y coordinates are restored when it's no longer centered.
        control.anchors.centerIn = undefined
        compare(centerInSpy.count, 5)
        currentPos = getCurrentPos()
        compare(currentPos.x, 33)
        compare(currentPos.y, 44)

        // Test centering in the overlay while having a different parent (anotherItem).
        control.anchors.centerIn = overlay
        compare(centerInSpy.count, 6)
        currentPos = getCurrentPos()
        // calculate the correct co-ordinates in the overlay's co-ordinate system
        let expectedX = (overlay.width - (control.width * control.scale)) / 2
        let expectedY = (overlay.height - (control.height * control.scale)) / 2
        // translate to the parent's (anotherItem's) co-ordinate system
        expectedX -= anotherItem.x
        expectedY -= anotherItem.y
        compare(currentPos.x, expectedX)
        compare(currentPos.y, expectedY)
    }

    Component {
        id: shortcutWindowComponent
        ApplicationWindow {
            id: window
            width: 360
            height: 360
            visible: true

            property alias popup: popup
            property alias shortcut: shortcut

            Popup {
                id: popup

                Shortcut {
                    id: shortcut
                    sequence: "A"
                    onActivated: popup.visible = !popup.visible
                }
            }
        }
    }

    function test_shortcut() {
        // Tests that a Shortcut with Qt.WindowShortcut context
        // that is declared within a Popup is activated.
        let window = createTemporaryObject(shortcutWindowComponent, testCase)
        let control = window.popup

        window.requestActivate()
        tryCompare(window, "active", true)

        let shortcutActivatedSpy = createTemporaryObject(signalSpy, testCase,
            { target: window.shortcut, signalName: "activated"} )
        verify(shortcutActivatedSpy.valid)

        waitForRendering(window.contentItem)
        keyClick(Qt.Key_A)
        compare(shortcutActivatedSpy.count, 1)
        tryCompare(control, "visible", true)

        keyClick(Qt.Key_A)
        compare(shortcutActivatedSpy.count, 2)
        tryCompare(control, "visible", false)
    }

    Component {
        id: mousePropagationComponent
        ApplicationWindow {
            id: window
            width: 360
            height: 360
            visible: true

            property alias popup: popup
            property alias popupTitle: popupTitle
            property alias popupContent: popupContent
            property bool gotMouseEvent: false

            MouseArea {
                id: windowMouseArea
                enabled: true
                anchors.fill: parent
                onPressed: gotMouseEvent = true
            }


            Popup {
                id: popup
                width: 200
                height: 200

                background: Rectangle {
                    id: popupContent
                    color: "#505050"
                    Rectangle {
                        id: popupTitle
                        width: parent.width
                        height: 30
                        color: "blue"

                        property point pressedPosition: Qt.point(0, 0)

                        MouseArea {
                            enabled: true
                            propagateComposedEvents: true
                            anchors.fill: parent
                            onPressed: (mouse) => {
                                popupTitle.pressedPosition  = Qt.point(mouse.x, mouse.y)
                            }
                            onPositionChanged: (mouse) => {
                                popup.x += mouse.x - popupTitle.pressedPosition.x
                                popup.y += mouse.y - popupTitle.pressedPosition.y
                            }
                            onReleased: (mouse) => {
                                popupTitle.pressedPosition = Qt.point(0, 0)
                            }
                        }
                    }
                }

                Component.onCompleted: {
                    x = parent.width / 2 - width / 2
                    y = parent.height / 2 - height / 2
                }
            }
        }
    }

    function test_mousePropagation() {
        // Tests that Popup ignores mouse events that it doesn't handle itself
        // so that they propagate correctly.
        let window = createTemporaryObject(mousePropagationComponent, testCase)
        if (window.popup.popupType === Popup.Window)
            skip("It is not necessary to test mouse propagation for Popup Window types.")
        window.requestActivate()
        tryCompare(window, "active", true)

        let popup = window.popup
        popup.open()
        tryCompare(popup, "opened", true)
        waitForRendering(popup.contentItem)

        // mouse clicks into the popup must not propagate to the parent
        mouseClick(window)
        compare(window.gotMouseEvent, false)

        let title = window.popupTitle
        verify(title)

        let pressPoint = Qt.point(title.width / 2, title.height / 2)
        let oldPos = Qt.point(popup.x, popup.y)
        mousePress(title, pressPoint.x, pressPoint.y)
        fuzzyCompare(title.pressedPosition.x, pressPoint.x, 1)
        fuzzyCompare(title.pressedPosition.y, pressPoint.y, 1)
        mouseMove(title, pressPoint.x + 5, pressPoint.y + 5)
        fuzzyCompare(popup.x, oldPos.x + 5, 1)
        fuzzyCompare(popup.y, oldPos.y + 5, 1)
        mouseRelease(title, pressPoint.x, pressPoint.y)
        compare(title.pressedPosition, Qt.point(0, 0))


        // Set modal as true and check for the same operation
        popup.modal = true
        oldPos = Qt.point(popup.x, popup.y)
        mousePress(title, pressPoint.x, pressPoint.y)
        fuzzyCompare(title.pressedPosition.x, pressPoint.x, 1)
        fuzzyCompare(title.pressedPosition.y, pressPoint.y, 1)
        mouseMove(title, pressPoint.x + 5, pressPoint.y + 5)
        fuzzyCompare(popup.x, oldPos.x + 5, 1)
        fuzzyCompare(popup.y, oldPos.y + 5, 1)
        mouseRelease(title, pressPoint.x, pressPoint.y)
        compare(title.pressedPosition, Qt.point(0, 0))
    }

    Component {
        id: cppDimmerComponent

        Popup {
            dim: true
            Overlay.modeless: ComponentCreator.createComponent(
                "import QtQuick; Rectangle { objectName: \"rect\"; color: \"tomato\" }")
        }
    }

    function test_dimmerComponentCreatedInCpp() {
        let control = createTemporaryObject(cppDimmerComponent, testCase)
        verify(control)

        control.open()
        tryCompare(control, "opened", true)
        let rect = findChild(control.Overlay.overlay, "rect")
        verify(rect)
    }

    Component {
        id: popupOverlay

        Popup {
            id: popupCenterIn
            property alias text: toastText.text
            anchors.centerIn: Overlay.overlay
            dim: true
            modal: true
            contentItem: ColumnLayout {
                Text {
                    id: toastText
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
            }
        }
    }

    function test_popupOverlayCenterIn() {
        if (Qt.platform.os === "linux") {
            // So far observed timeout in Imagine and FluentWinUI3 style
            skip("This function times out with some styles on RHEL 8.10")
        }
        let control = createTemporaryObject(popupOverlay, testCase)
        verify(control)

        // Modify text in the popup content item
        control.text = "this is a long text causing a line break to show the binding loop "
                        + "(height) again after the first initialization of the text";

        // Open popup item and wait for it to be rendered
        control.open()
        waitForRendering(testCase)
        tryCompare(control, "opened", true)

        // Verify popup position
        if (control.popupType === Popup.Window) {
            // popup windows don't have edge constraints.
            tryVerify(function(){ return control.x < 0 })
            compare(control.y, Math.round(control.parent.height / 2 - control.height / 2))
        } else {
            // popup items have edge constraints.
            compare(control.x, 0)
            compare(control.y, control.parent.height / 2 - control.height / 2)
        }
        control.close()
    }

    Component {
        id: popupWithOverlayInLoader

        Loader {
            id: loader
            active: false
            sourceComponent: Item {
                anchors.fill: parent
                Popup {
                    modal: true
                    visible: true
                    Overlay.modal: Rectangle { color: 'grey' }
                }
            }
        }
    }

    function test_popupWithOverlayInLoader() { // QTBUG-122915
        let loader = createTemporaryObject(popupWithOverlayInLoader, testCase)
        verify(loader)

        let overlay = loader.Overlay.overlay
        verify(overlay)

        loader.active = true
        tryCompare(overlay, "visible", true)

        loader.active = false
        tryCompare(overlay, "visible", false)
    }
}
