// Copyright (c) 2014-2024, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import QtQuick 2.9
import QtMultimedia 5.4
import QtQuick.Dialogs 1.2
import UrOts 0.1

Rectangle {
    id : root

    x: 0
    y: 0
    z: parent.z+1
    width: parent.width
    height: parent.height

    visible: false
    color: "black"
    state: "Stopped"
    Shortcut {
        sequence: StandardKey.Cancel  // This includes the Escape key
        onActivated: {
            root.state = "Stopped"
        }
    }

    states: [
        State {
            name: "Display"
            StateChangeScript {
                script: {
                    root.visible = true
                }
            }
        },
        State {
            name: "Stopped"
            StateChangeScript {
                script: {
                    root.visible = false
                }
            }
        }
    ]

    function setData(urtype, data) {
        urWidget.setData(urtype, data)
    }

    UrWidget {
        id : urWidget
        objectName: "UrWidget"
        visible: root.state == "Capture"

        x: 0
        y: 0
        //z: parent.z+1
        width: min(parent.width, parent.height)
        height: min(parent.width, parent.height)

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            onDoubleClicked: {
                root.state = "Stopped"
            }
        }
    }
}
