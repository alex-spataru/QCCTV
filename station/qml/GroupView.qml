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
import QtQuick.Controls 2.0

GridView {
    //
    // Propeties
    //
    property int group: 0
    property bool enabled: true

    //
    // Resize grid cells when user resizes window
    //
    onGroupChanged: redraw()
    onWidthChanged: redraw()
    onHeightChanged: redraw()

    //
    // Scrollbars
    //
    ScrollIndicator.vertical: ScrollIndicator { }
    ScrollIndicator.horizontal: ScrollIndicator { }

    //
    // Re-sizes the cells to fit the application window size
    //
    function redraw() {
        /* Reset the model */
        model = 0
        model = enabled ? QCCTVStation.cameraCount (group) : 0

        /* Initialize variables */
        var cols = 1
        var rows = 1
        var toggler = false

        /* Calculate ideal rows and columns size */
        while (cols * rows < model) {
            if (toggler)
                rows += 1
            else
                cols += 1

            toggler = !toggler
        }

        /* Force the UI to generate a 2x2 grid when having 2 cams */
        if (model === 2) {
            rows = 2
            cols = 2
        }

        /* Get available size */
        var w = width
        var h = height

        /* Set initial size */
        cellWidth = w / Math.max (rows, 1)
        cellHeight = h / Math.max (cols, 1)

        /* Ensure that cells are not too small */
        cellWidth = Math.max (cellWidth, Math.max (w / 12, 140))
        cellHeight = Math.max (cellHeight, Math.max (h / 9, 100))
    }

    //
    // Fade while hidding or showing
    //
    opacity: enabled ? 1 : 0
    Behavior on opacity { NumberAnimation {} }

    //
    // Camera view object
    //
    delegate: Camera {
        id: cam
        clip: false
        enabled: enabled
        width: cellWidth
        height: cellHeight
        onClicked: fullscreenCamera.showCamera (camNumber)
        camNumber: QCCTVStation.getGroupCameraIDs (group) [index]
    }
}
