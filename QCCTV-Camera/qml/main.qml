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
import QtQuick.Controls 1.0
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0

import "qrc:/common/"

ApplicationWindow {
    id: app
    
    //
    // Window geometry
    //
    width: 720
    height: 480
    x: isMobile ? 0 : 100
    y: isMobile ? 0 : 100

    //
    // Window properties
    //
    color: "#000"
    visible: true
    title: AppDspName + " " + AppVersion

    //
    // Global variables
    //
    property var spacing: 8
    property int forceReload: 0
    property bool controlsEnabled: true

    //
    // Settings
    //
    Settings {
        property alias x: app.x
        property alias y: app.y
        property alias width: app.width
        property alias height: app.height
    }

    //
    // Reloads the displayed image by changing the URL to load the images
    // from (which forces the image to perform a redraw)
    //
    function reloadImage() {
        image.source = "image://qcctv/reload"
        image.source = "image://qcctv/"
        image.sourceChanged (image.source)
    }

    //
    // Show window correctly on mobile devices
    //
    Component.onCompleted: {
        if (isMobile)
            showMaximized()
    }

    //
    // QCCTV signals/slots
    //
    Connections {
        target: QCCTVCamera

        onImageChanged: reloadImage()
        onFpsChanged: fps.text = QCCTVCamera.fps() + " FPS"
        onCameraNameChanged: camName.text = QCCTVCamera.cameraName()
        onFocusStatusChanged: status.display (qsTr ("Focusing Camera") + "...")

        onLightStatusChanged: {
            if (QCCTVCamera.flashlightOn())
                status.display (qsTr ("Flashlight enabled"))
            else
                status.display (qsTr ("Flashlight disabled"))
        }
    }

    //
    // Video output
    //
    Image {
        id: image
        cache: false
        smooth: true  
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop

        MouseArea {
            anchors.fill: parent
            onClicked: app.controlsEnabled = !app.controlsEnabled
        }

        Behavior on opacity { NumberAnimation{} }
    }

    //
    // Blur image when settings panel is shown
    //
    FastBlur {
        id: blur
        radius: 0
        source: image
        anchors.fill: parent

        Behavior on radius { NumberAnimation {} }
    }

    //
    // Top status bar
    //
    Panel {
        id: menu
        height: 24 + spacing
        opacity: app.controlsEnabled ? 1 : 0

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: app.spacing
        }

        //
        // Camera name
        //
        Label {
            id: camName
            text: QCCTVCamera.cameraName()

            anchors {
                left: parent.left
                margins: spacing
                verticalCenter: parent.verticalCenter
            }
        }

        //
        // FPS indicator
        //
        Label {
            id: fps
            text: QCCTVCamera.fps() + " FPS"

            anchors {
                right: parent.right
                margins: spacing
                verticalCenter: parent.verticalCenter
            }
        }
    }

    //
    // Action buttons
    //
    Row {
        id: buttons
        spacing: app.spacing * 2
        opacity: app.controlsEnabled ? 1 : 0

        Behavior on opacity { NumberAnimation{} }

        anchors {
            bottom: parent.bottom
            margins: app.spacing * 2
            horizontalCenter: parent.horizontalCenter
        }

        //
        // Light control button
        //
        Button {
            width: 54
            height: 54
            enabled: app.controlsEnabled
            source: "qrc:/common/images//light.png"
            anchors.verticalCenter: parent.verticalCenter

            onClicked: {
                toggled = !toggled

                QCCTVCamera.turnOffFlashlight()

                if (QCCTVCamera.flashlightAvailable()) {
                    if (toggled)
                        QCCTVCamera.turnOnFlashlight()
                    else
                        QCCTVCamera.turnOffFlashlight()
                }

                else
                    status.display (qsTr ("Flashlight Error"))
            }
        }

        //
        // Settings button
        //
        Button {
            width: 64
            height: 64
            onClicked: settings.showPanel()
            enabled: app.controlsEnabled
            source: "qrc:/common/images//settings.png"
            anchors.verticalCenter: parent.verticalCenter
        }

        //
        // Photo button
        //
        Button {
            width: 64
            height: 64
            enabled: app.controlsEnabled
            source: "qrc:/common/images//camera.png"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                if (QCCTVCamera.readyForCapture()) {
                    status.display (qsTr ("Saving Photo") + "...")
                    QCCTVCamera.takePhoto()
                }

                else
                    status.display (qsTr ("Camera not Ready!"))
            }
        }

        //
        // Focus button
        //
        Button {
            width: 54
            height: 54
            enabled: app.controlsEnabled
            source: "qrc:/common/images//focus.png"
            onClicked: QCCTVCamera.focusCamera()
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    //
    // Status label
    //
    Panel {
        id: status
        height: 24
        opacity: 0
        width: Math.min (app.width * 0.6, sText.width * 2)
        
        anchors {
            bottom: parent.bottom
            bottomMargin: buttons.height + 6 * spacing
            horizontalCenter: image.horizontalCenter
        }

        //
        // Shows the status window with the given \a text
        // for two seconds and then hides the window
        //
        function display (text) {
            sTimer.restart()
            sText.text = text
            status.opacity = 1
        }

        //
        // Window hide timer
        //
        Timer {
            id: sTimer
            interval: 2000
            onTriggered: status.opacity = 0
        }

        //
        // Dynamic status label
        //
        Label {
            id: sText
            anchors.centerIn: parent
        }
    }

    //
    // Settings panel
    //
    Item {
        id: settings
        Component.onCompleted: settings.hidePanel()

        //
        // Resize the margins when window size changes
        //
        Connections {
            target: app
            onWidthChanged: {
                if (settings.anchors.leftMargin > 0)
                    settings.anchors.leftMargin = app.width
            }
        }

        //
        // Shows the settings dialog
        //
        function showPanel() {
            blur.radius = 64
            settings.opacity = 1
            anchors.leftMargin = 0
            controlsEnabled = false
        }

        //
        // Hides the settings dialog
        //
        function hidePanel() {
            blur.radius = 0
            settings.opacity = 0
            controlsEnabled = true
            anchors.leftMargin = app.width
        }

        //
        // Geometry
        //
        anchors.fill: parent
        anchors.leftMargin: parent.width

        //
        // Animations
        //
        Behavior on opacity { NumberAnimation{} }
        Behavior on anchors.leftMargin { NumberAnimation{} }

        //
        // Hide the panel when clicking on it
        //
        MouseArea {
            anchors.fill: parent
            enabled: anchors.leftMargin === 0
            onClicked: settings.hidePanel()
        }

        //
        // Back button
        //
        Panel {
            id: back
            width: settingsLabel.height
            height: settingsLabel.height

            anchors {
                top: parent.top
                left: parent.left
                margins: app.spacing
            }

            Image {
                anchors.centerIn: parent
                sourceSize: Qt.size (18, 18)
                source: "qrc:/common/images/back.png"
            }

            MouseArea {
                anchors.fill: parent
                enabled: settings.opacity > 0
                onClicked: settings.hidePanel()
            }
        }

        //
        // Settings label
        //
        Panel {
            id: settingsLabel
            height: 24 + spacing

            anchors {
                top: parent.top
                left: back.right
                right: parent.right
                margins: app.spacing
            }

            Label {
                text: qsTr ("Settings")

                anchors {
                    left: parent.left
                    margins: spacing
                    verticalCenter: parent.verticalCenter
                }
            }
        }

        //
        // Controls
        //
        Panel {
            anchors.fill: parent
            anchors.margins: app.spacing
            anchors.topMargin: settingsLabel.height + (2 * app.spacing)
        }
    }
}
