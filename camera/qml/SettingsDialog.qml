/*
 * Copyright (c) 2016 Alex Spataru
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE
 */

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import Qt.labs.settings 1.0
import QtQuick.Controls.Material 2.0

Popup {
    id: settings

    //
    // Settings
    //
    Settings {
        property alias fps: fps.value
        property alias name: name.text
        property alias group: group.text
        property alias resolution: resolutions.currentIndex
        property alias autoRegulateResolution: autoRegulateResolution.checked
    }

    //
    // Geometry options
    //
    modal: true
    contentWidth: col.width
    contentHeight: col.height
    x: (app.width - width) / 2
    y: (app.height - height * 1.2) / 2

    //
    // Styling options
    //
    Material.theme: Material.Light

    //
    // Central widget
    //
    ColumnLayout {
        id: col
        spacing: app.spacing
        anchors.centerIn: parent

        //
        // Camera name label
        //
        Label {
            text: qsTr ("Camera Name") + ":"
        }

        //
        // Camera name text input
        //
        TextField {
            id: name
            text: QCCTVCamera.name
            Layout.fillWidth: true
            Layout.minimumWidth: 280
            onTextChanged: {
                if (text.length > 0)
                    QCCTVCamera.name = text
            }
        }

        //
        // Camera group label
        //
        Label {
            text: qsTr ("Camera Group") + ":"
        }

        //
        // Camera group text input
        //
        TextField {
            id: group
            Layout.fillWidth: true
            text: QCCTVCamera.group
            Layout.minimumWidth: 280
            onTextChanged: {
                if (text.length > 0)
                    QCCTVCamera.group = text
            }
        }

        //
        // FPS label
        //
        Label {
            text: qsTr ("Camera FPS") + ":"
        }

        //
        // FPS spinbox
        //
        SpinBox {
            id: fps
            value: QCCTVCamera.fps
            Layout.fillWidth: true
            to: QCCTVCamera.maximumFps
            from: QCCTVCamera.minimumFps
            onValueChanged: QCCTVCamera.fps = value
        }

        //
        // Resolution label
        //
        Label {
            text: qsTr ("Target Resolution") + ":"
        }

        //
        // Resolution selector
        //
        ComboBox {
            id: resolutions
            Layout.fillWidth: true
            model: QCCTVCamera.resolutions
            currentIndex: QCCTVCamera.resolution
            onCurrentIndexChanged: QCCTVCamera.resolution = currentIndex
        }

        //
        // Auto-regulate switch
        //
        Switch {
            id: autoRegulateResolution
            checked: QCCTVCamera.autoRegulateResolution
            text: qsTr ("Auto-regulate video resolution")
            onCheckedChanged: QCCTVCamera.autoRegulateResolution = checked
        }

        //
        // Spacer
        //
        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: app.spacing
        }

        //
        // Close button
        //
        Button {
            text: qsTr ("Close")
            Layout.fillWidth: true
            onClicked: settings.close()
        }
    }
}
