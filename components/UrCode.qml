import QtQuick 2.9
import QtMultimedia 5.4
import OtsUr 0.1
import "." as MoneroComponents

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
    states: [
        State {
            name: "Display"
            StateChangeScript {
                script: {
                    root.visible = true
                    root.focus = true
                    textDisplayType.text = "UR Code"
                }
            }
        },
        State {
            name: "Stopped"
            StateChangeScript {
                script: {
                    root.visible = false
                    root.focus = false
                    textDisplayType.text = ""
                }
            }
        }
    ]

    Image {
        id: qrCodeImage
        cache: false
        width: qrCodeImage.height
        height: Math.max(300, Math.min(parent.height - frameInfo.height - displayType.height - 240, parent.width - 40))
        anchors.centerIn: parent
        function reload() {
            var tmp = qrCodeImage.source
            qrCodeImage.source = ""
            qrCodeImage.source = tmp
        }
    }

    Rectangle {
        id: frameInfo
        height: textFrameInfo.height + 5
        width: textFrameInfo.width + 20
        z: parent.z + 1
        radius: 16
        color: "#FA6800"
        visible: textFrameInfo.text !== ""
        anchors.centerIn: textFrameInfo
        opacity: 0.4
    }

    Text {
        id: textFrameInfo
        z: frameInfo.z + 1
        visible: urSender.isUrCode
        text: urSender.currentFrameInfo
        anchors.top: parent.top
        anchors.horizontalCenter: qrCodeImage.horizontalCenter
        anchors.margins: 30
        font.pixelSize: 22
        color: "white"
        opacity: 0.7
    }

    Rectangle {
        id: displayType
        height: textDisplayType.height + 5
        width: textDisplayType.width + 20
        z: parent.z + 1
        radius: 16
        color: "#FA6800"
        anchors.centerIn: textDisplayType
        opacity: 0.4
    }

    Text {
        id: textDisplayType
        z: displayType.z + 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: btnClose.top
        anchors.margins: 30
        text: ""
        font.pixelSize: 22
        color: "white"
        opacity: 0.7
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

    function setData(type, data) {
        urSender.setData(type, data)
    }

    Component.onCompleted: {
        qrCodeImage.source = "image://urcode/qr"
    }
}
