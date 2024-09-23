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

    readonly property var walletModes: {
        "Both": 3,
        "Full": 2,
        "ViewOnly": 1
    }

    readonly property var qrCodeFormats: {
        "Both": 3,
        "Uri": 1,
        "Json": 2
    }

    readonly property var addressTypes: {
        "Any": 3,
        "Wallet": 1,
        "SubAddress": 2
    }

    readonly property var transactionFormats: {
        "Both": 3,
        "Unsigned": 1,
        "Signed": 2
    }

    readonly property var modes: {
        "None": 0,
        "QrCode": 1,
        "Wallet": 2,
        "Address": 3,
        "Transaction": 4,
        "Outputs": 5,
        "KeyImages": 6
    }

    property int mode: modes.None
    property int qrCodeFormat: qrCodeFormats.Both
    property int walletMode: walletModes.Both
    property int addressType: addressTypes.Any
    property int transactionFormat:  transactionFormats.Both

    signal qrcode_decoded(string address, string payment_id, string amount, string tx_description, string recipient_name, var extra_parameters)
    signal canceled()
    signal qrCode(string data)
    signal wallet(string address, string spendKey, string viewKey, int height)
    signal viewWallet(string address, string viewKey, int height)
    signal transaction(string tx, int txFormat)
    signal address(string address, string payment_id, string recipient_name, string amount, string description)
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
                    urScanner.reset()
                    urScanner.startCapture(root.mode in [ root.modes.Outputs, root.modes.KeyImages, root.modes.Transaction ])
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
                    root.qrCodeFormat = qrCodeFormats.Both
                    root.walletMode = walletModes.Both
                    root.addressType = addressTypes.Any
                    root.transactionFormat = transactionFormats.Both
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
            console.warn("onQrDataReceived: " + data)
            if(!root.mode in [ root.modes.QrCode, root.modes.Wallet, root.modes.Address ]) {
                urScanner.reset()
                return
            }
            switch(root.mode) {
            case root.modes.Wallet:
                var w = MoneroUri.parseWalletDataUri(data)
                if(!w) {
                   root.cancel()
                    return
                }
                if(root.walletMode in [root.walletModes.Both, root.walletModes.Full])
                    root.wallet(w.address, w.spendKet, w.viewKey, w.height)
                if(root.walletMode in [root.walletModes.Both, root.walletModes.ViewOnly])
                    root.viewWallet(w.address, w.viewKey, w.height)
                break
            case root.modes.Address:
                var t = MoneroUri.parseTxDataUri(data)
                if(!t) {
                   root.cancel()
                    return
                }
                root.address(t.address, t.paymentId, t.recipientName, t.amount, t.description)
                break
            case root.modes.QrCode:
                root.qrCode(data)
                break
            default:
                root.cancel()
                return
            }
            root.mode = root.modes.None
        }

        onUrDataReceived: function(type, data) {
            if(!root.mode in [ root.modes.Outputs, root.modes.KeyImages, root.modes.Transaction ]) {
                root.cancel() // should never happen
                return
            }
            switch(root.mode) {
            case root.modes.Outputs:
                break
            case root.modes.KeyImages:
                break
            case root.modes.Transaction:
                break
            default:
                root.cancel()
                return
            }
            root.mode = root.modes.None
        }

        onUrDataFailed: function(error) {
            if(!root.mode in [ root.modes.Outputs, root.modes.KeyImages, root.modes.Transaction ])
                return // should never happen
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
        onDecoded : {
            const parsed = walletManager.parse_uri_to_object(data);
            if (!parsed.error) {
                root.qrcode_decoded(parsed.address, parsed.payment_id, parsed.amount, parsed.tx_description, parsed.recipient_name, parsed.extra_parameters);
                root.state = "Stopped";
            } else if (walletManager.addressValid(data, appWindow.persistentSettings.nettype)) {
                root.qrcode_decoded(data, "", "", "", "", null);
                root.state = "Stopped";
            } else {
                onNotifyError(parsed.error);
            }
        }
    }
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

    Component.onCompleted: {
        if( QtMultimedia.availableCameras.length === 0) {
            console.warn("No camera available. Disable qrScannerEnabled")
            appWindow.qrScannerEnabled = false
        }
    }
}
