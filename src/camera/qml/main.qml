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
import QtMultimedia 5.4
import QtQuick.Controls 2.0

import "."

ApplicationWindow {
    id: app
    width: 720
    height: 480
    color: "#111"
    visible: true
    title: AppDspName + " " + AppVersion

    property var borderSize: 8

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVCamera
        onFpsChanged: fps.text = QCCTVCamera.fps() + " FPS"
        onCameraNameChanged: camName.text = QCCTVCamera.cameraName()
        onFocusStatusChanged: status.display (qsTr ("Focusing Camera") + "...")
    }

    //
    // Top status bar
    //
    Rectangle {
        id: menu
        color: "#000"
        height: 24 + borderSize

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        //
        // Camera name
        //
        Label {
            id: camName
            color: "white"
            font.family: "OpenSans"
            text: QCCTVCamera.cameraName()

            anchors {
                left: parent.left
                margins: borderSize
                verticalCenter: parent.verticalCenter
            }
        }

        //
        // FPS indicator
        //
        Label {
            id: fps
            color: "white"
            font.family: "OpenSans"
            text: QCCTVCamera.fps() + " FPS"

            anchors {
                right: parent.right
                margins: borderSize
                verticalCenter: parent.verticalCenter
            }
        }
    }

    //
    // Video output
    //
    VideoOutput {
        id: video

        source: Camera {
            captureMode: Camera.CaptureStillImage
        }

        anchors.fill: parent
        autoOrientation: true
        anchors.topMargin: menu.height
        anchors.rightMargin: panel.toggled ? panel.width : 0

        Behavior on anchors.rightMargin { NumberAnimation{} }

        //
        // Panel togglder
        //
        MouseArea {
            anchors.fill: parent
            onClicked: panel.toggled = !panel.toggled
        }
    }

    //
    // Status label
    //
    Rectangle {
        id: status

        radius: 2
        height: 24
        opacity: 0
        width: Math.min (app.width * 0.6, sText.width * 2)

        color: "#222"

        function display (text) {
            sTimer.start()
            sText.text = text
            status.opacity = 0.8
        }

        Behavior on opacity { NumberAnimation{} }

        Timer {
            id: sTimer
            interval: 2000
            onTriggered: status.opacity = 0
        }

        Label {
            id: sText
            color: "white"
            font.family: "OpenSans"
            anchors.centerIn: parent
        }

        anchors {
            top: parent.top
            topMargin: menu.height + borderSize
            horizontalCenter: video.horizontalCenter
        }
    }

    //
    // Panel
    //
    Rectangle {
        id: panel
        color: "#222"
        width: 72

        property var toggled: true

        anchors {
            top: menu.bottom
            right: parent.right
            bottom: parent.bottom
            rightMargin: toggled ? 0 : -2  * width
        }

        Behavior on anchors.rightMargin { NumberAnimation{} }

        //
        // Actions column
        //
        Column {
            anchors.fill: parent
            spacing: borderSize * 2
            anchors.margins: borderSize * 2
            anchors.bottomMargin: settings.height

            Button {
                source: "qrc:/images/focus.png"
                onClicked: QCCTVCamera.focusCamera()
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Button {
                onClicked: toggled = !toggled
                source: "qrc:/images/light.png"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        //
        // Settings Button
        //
        Button {
            id: settings
            source: "qrc:/images/settings.png"
            anchors.horizontalCenter: parent.horizontalCenter

            anchors {
                bottom: parent.bottom
                margins: borderSize * 2
                horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
