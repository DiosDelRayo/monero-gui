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
    signal unsignedTx(var tx)
    signal signedTx(var tx)
    signal keyImages(var keyImages)
    signal outputs(var outputs)

    states: [
        State {
            name: "Capture"
            when: root.mode !== root.modes.None
            StateChangeScript {
                script: {
                    root.visible = true
                    for(var i = 0; i < QtMultimedia.availableCameras.length; i++)
                        if(QtMultimedia.availableCameras[i].deviceId === persistentSettings.lastUsedCamera) {
                            urCamera.deviceId = persistentSettings.lastUsedCamera
                            break
                        }
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
            persistentSettings.lastUsedCamera = urCamera.deviceId
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
        onChanged: {
            urCamera.deviceId = QtMultimedia.availableCameras[cameraChooser.currentIndex].deviceId
            persistentSettings.lastUsedCamera = urCamera.deviceId
        }
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
            id: scanTypeFrame
            height: scanType.height + 20
            width: scanType.width + 30
            z: parent.z + 1
            radius: 16
            color: "#FA6800"
            opacity: 0.4
            anchors.centerIn: scanType
        }

        Text {
            z: scanTypeFrame.z + 1
            anchors.centerIn: parent
            id: scanType
            text: ""
            font.pixelSize: 22
            color: "white"
            opacity: 0.7
        }

        Rectangle {
            id: scanProgress
            property int scannedFrames: 0
            property int totalFrames: 0
            property int progress: 0
            visible: true
            height: textScanProgress.height + 10
            width: viewfinder.width - 40
            z: viewfinder.z + 1
            radius: 16
            color: "#FA6800"
            opacity: 0.4
            anchors.centerIn: viewfinder
            anchors.bottom: viewfinder.bottom
            anchors.bottomMargin: 20
            function onScannedFrames(count, total) {
                console.warn("scanned frames: " + count + "/" + total)
                scanProgress.scannedFrames = count
                scanProgress.totalFrames = total
            }
            function onProgress(complete) {
                console.warn("progress: " + (complete * 100) + "%")
                scanProgress.progress = complete * 100
            }
            function reset() {
                scanProgress.progress = 0
                scanProgress.scannedFrames = 0
                scanProgress.totalFrames = 0
            }
        }

        Rectangle {
            id: scanProgressBar
            visible: scanProgressBar.width > 32
            height: scanProgress - 4
            width: (parent.width - 4) * 100 / scanProgress.progress
            x: parent.x + 2
            y: parent.y + 2
            radius: 16
        }

        Text {
            z: scanProgress.z + 2
            anchors.centerIn: parent
            id: textScanProgress
            text: (scanProgress.progress > 0 || scanProgress.totalFrames > 0) ? (scanProgress.progress + "% (" + scanProgress.scannedFrames + "/" + scanProgress.totalFrames + ")") : ""
            font.pixelSize: 22
            color: "white"
            opacity: 0.7
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

    function onUnexpectedFrame(urType){
        console.warn("unexpected type: " + urType);
    }

    function onReceivedFrames(count){
        console.warn("frame count: " + count);
    }

    function onDecodedFrame(data){
        console.warn("decoded frame: " + data);
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
        urScanner.qrCaptureStarted(scanProgress.reset)
        urScanner.scannedFrames(scanProgress.onScannedFrames)
        urScanner.estimatedCompletedPercentage(scanProgress.onProgress)
        urScanner.unexpectedUrType(root.onUnexpectedFrame)
        urScanner.receivedFrames(root.onReceivedFrames)
        urScanner.decodedFrame(root.onDecodedFrame)
    }
}
