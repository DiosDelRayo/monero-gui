import QtQuick 2.9
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
        visible: root.state == "Display"

        x: 0
        y: 0
        //z: parent.z+1
        width: 200 //min(parent.width, parent.height)
        height: 200 //min(parent.width, parent.height)

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            onDoubleClicked: {
                root.state = "Stopped"
            }
        }
    }
    Component.onStatusChanged: function(status) { console.log("status changed: " + status) }
    Component.onCompleted: {
        console.log("Yeah urDisplay completed")
        urWidget.setData("test", "test")
    }
}
