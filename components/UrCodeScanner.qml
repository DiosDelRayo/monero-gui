import QtQuick 2.9
import QtQml.Models 2.2
import QtMultimedia 5.4
import QtQuick.Dialogs 1.2
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

    readonly property var modes: {
        "None": 0,
        "QrCode": 1,
        "Wallet": 2,
        "Address": 3,
        "Outputs": 4,
        "KeyImages": 5,
        "UnsignedTx": 6,
        "SignedTx": 7
    }

    property int mode: modes.None

    signal canceled()
    signal qrCode(string data)
    signal wallet(MoneroWalletData walletData)
    signal txData(MoneroTxData data)
    signal unsignedTx(string tx)
    signal signedTx(string tx)
    signal keyImages(string keyImages)
    signal outputs(string outputs)

    states: [
        State {
            name: "Capture"
            when: root.mode !== root.modes.None
            StateChangeScript {
                script: {
                    console.warn("script: capture")
                    root.visible = true
                    urCamera.captureMode = Camera.CaptureStillImage
                    urCamera.cameraState = Camera.ActiveState
                    urCamera.start()
                }
            }
        },
        State {
            name: "Stopped"
            when: root.mode === root.modes.None
            StateChangeScript {
                script: {
                    console.warn("script: stopped")
                    urCamera.stop()
                    urScanner.stop()
                    root.visible = false
                    urCamera.cameraState = Camera.UnloadedState
                }
            }
        }
    ]

    ListModel {
        id: availableCameras
        Component.onCompleted: {
            availableCameras.clear()
            for(var i = 0; i < QtMultimedia.availableCameras.length; i++) {
                var cam = QtMultimedia.availableCameras[i]
                availableCameras.append({
                                            column1: cam.displayName,
                                            column2: cam.deviceId,
                                            priority: i
                                        })
            }
        }
    }

    UrCodeScannerImpl {
        id: urScanner
        objectName: "urScanner"
        onQrDataReceived: function(data) {
            root.mode = root.modes.None
        }

        onUrDataReceived: function(type, data) {
            root.mode = root.modes.None
        }

        onUrDataFailed: function(error) {
            root.cancel()
        }
    }

    MoneroComponents.StandardButton {
        id: btnSwitchCamera
        visible: QtMultimedia.availableCameras.length === 2 // if the system has exact to cams, show a switch button
        text: qsTr("Switch Camera")
        z: viewfinder.z + 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        onClicked: {
            btnSwitchCamera.visible = false
            urCamera.deviceId = urCamera.deviceId === QtMultimedia.availableCameras[0].deviceId ? QtMultimedia.availableCameras[1].deviceId : QtMultimedia.availableCameras[0].deviceId
            btnSwitchCamera.visible = true
        }
    }

    StandardDropdown {
        id: cameraChooser
        visible: QtMultimedia.availableCameras.length > 2 // if the system has more then 2 cams, show a list
        z: viewfinder.z + 1
        width: 300
        height: 30
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        dataModel: availableCameras
        onChanged: urCamera.deviceId = QtMultimedia.availableCameras[cameraChooser.currentIndex].deviceId
    }

    Camera {
        id: urCamera
        objectName: "urCamera"
        captureMode: Camera.CaptureStillImage
        cameraState: Camera.UnloadedState

        focus {
            focusMode: Camera.FocusContinuous
        }
    }

    /*
    QRCodeScanner {
            const parsed = walletManager.parse_uri_to_object(data);
                root.qrcode_decoded(parsed.address, parsed.payment_id, parsed.amount, parsed.tx_description, parsed.recipient_name, parsed.extra_parameters);
            } else if (walletManager.addressValid(data, appWindow.persistentSettings.nettype)) {
    */

    VideoOutput {
        id: viewfinder
        visible: root.state == "Capture"

        x: 0
        y: btnSwitchCamera.height + 40 // 2 x 20 (margin)
        z: parent.z+1
        width: parent.width
        height: parent.height - btnClose.height - btnSwitchCamera.height - 80 // 4 x 20 (margin)

        source: urCamera
        autoOrientation: true
        focus: visible

        MouseArea {
            anchors.fill: parent
            //propagateComposedEvents: true
            onPressAndHold: {
                if (camera.lockStatus === Camera.locked)camera.unlock()
                camera.searchAndLock()
            }
            onDoubleClicked: root.cancel()
        }
        Rectangle {
            height: 30
            width: 200
            z: parent.z + 1
            color: "orange"
            opacity: 0.4
            anchors.centerIn: parent
            Text {
                anchors.fill: parent
                id: scanType
                text: ""
                font.pixelSize: 24
                color: "black"
                opacity: 0.7
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: true
    }

    MoneroComponents.StandardButton {
        id: btnClose
        text: qsTr("Cancel")
        z: viewfinder.z + 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        anchors.topMargin: 20
        onClicked: root.cancel()
    }

    function cancel() {
        root.mode = root.modes.None
        root.canceled()
    }

    function scanQrCode() {
        root.mode = root.modes.QrCode
        scanType.text = qsTr("Scan QR Code")
        urScanner.qr()
    }

    function scanWallet() {
        root.mode = root.modes.Wallet
        scanType.text = qsTr("Scan Wallet QR Code")
        urScanner.scanWallet()
    }

    function scanTxData() {
        root.mode = root.modes.Address
        scanType.text = qsTr("Scan Tx Data QR Code")
        urScanner.scanTxData()
    }

    function scanOutputs() {
        root.mode = root.modes.Outputs
        scanType.text = qsTr("Scan Outputs UR Code")
        urScanner.scanOutputs()
    }

    function scanKeyImages() {
        root.mode = root.modes.KeyImages
        scanType.text = qsTr("Scan Key Images UR Code")
        urScanner.scanKeyImages()
    }

    function scanUnsignedTx() {
        root.mode = root.modes.UnsignedTx
        scanType.text = qsTr("Scan Unsigned Transaction UR Code")
        urScanner.scanUnsignedTx()
    }

    function scanSignedTx() {
        root.mode = root.modes.SignedTx
        scanType.text = qsTr("Scan Signed Transaction UR Code")
        urScanner.scanSignedTx()
    }

    Component.onCompleted: {
        if( QtMultimedia.availableCameras.length === 0) {
            console.warn("No camera available. Disable qrScannerEnabled")
            appWindow.qrScannerEnabled = false
            return
        }
        urScanner.outputs.connect(root.outputs)
        urScanner.keyImages.connect(root.keyImages)
        urScanner.unsignedTx.connect(root.unsignedTx)
        urScanner.signedTx.connect(root.signedTx)
        urScanner.txData.connect(root.txData)
        urScanner.wallet.connect(root.wallet)
    }
}
