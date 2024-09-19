import QtQuick 2.9
import QtMultimedia 5.4
import OtsUr 0.1
import "../components" as MoneroComponents

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
            console.warn('ESC')
            root.state = "Stopped"
        }
    }
    states: [
        State {
            name: "Display"
            StateChangeScript {
                script: {
                    console.warn('Display')
                    root.visible = true
                    root.focus = true
                }
            }
        },
        State {
            name: "Stopped"
            StateChangeScript {
                script: {
                    console.warn('Stopped')
                    root.visible = false
                    root.focus = false
                }
            }
        }
    ]

    /*
    function setData(urtype, data) {
        urSender.setData(urtype, data)
    }
    */
    Image {
        id: qrCodeImage
        cache: false
        width: 300
        height: 300
        anchors.centerIn: parent
        function reload() {
            var tmp = qrCodeImage.source
            qrCodeImage.source = ""
            qrCodeImage.source = tmp
        }
    }

    Text {
        id: textFrameInfo
        width: 150
        text: urSender.currentFrameInfo
        anchors.top: qrCodeImage.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: qrCodeImage.horizontalCenter
        verticalAlignment: verticalCenter
        color: "white"
    }

    MoneroComponents.StandardButton {
        id: btnClose
        text: qsTr("Close")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.topMargin: 20
        onClicked: root.state = "Stopped"
    }

    Connections {
        target: urSender
        function onUpdateQrCode() {
            qrCodeImage.reload()
        }
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        onDoubleClicked: {
            root.state = "Stopped"
        }
    }

    // Component.onStatusChanged: function(status) { console.log("status changed: " + status) }
    Component.onCompleted: {
        //urSender.setData("test-test", "testfsgregerhrekhmerwkhnwerklgnlwkengklwenhlekwnhwelrknhklrnhklernhklrenhlkernhklrenhklernhklerhnklerhnrkenh")
        urSender.onSettingsChanged(150, 80, true)
        urSender.test()
        qrCodeImage.source = "image://urcode/qr"
    }
}
