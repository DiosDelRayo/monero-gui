import QtQuick
import UrOts 0.1

Item {
    id: urScanComponent

	width: 200
	height: 200

	property alias decodedString: scanWidget.decodedString

    signal finished(bool success)
    signal urDataReceived(string type, string data)
    signal urDataFailed(string errorMsg)
    signal qrData(string data)
    signal urCaptureStarted();
    signal qrCaptureStarted();
    signal permissionError(string text)
    signal cameraSwitched(int index)
    signal scannedFrames(int count, int total)
    signal estimatedCompletedPercentage(float complete)
    
    function getURData() {
        return scanWidget.getURData()
    }

	function getURType() {
		return scanWidget.getURType()
	}

	function getURError() {
		return scanWidget.getURError()
	}

	function startCapture(scan_ur) {
		scanWidget.startCapture(scan_ur)
	}

	function reset() { scanWidget.reset() }
	function stop() { scanWidget.stop() }
	function pause() { scanWidget.pause() }

    ScanWidget {
        id: scanWidget
        anchors.fill: parent

		Component.onFinished: function(success) {
			urScanComponent.finished(success)
		}

		Component.onUrDataReceived: function(type, data) {
			urScanComponent.urDataReceived(type, data)
		}

		Component.onUrDataFailed: function(error) {
			urScanComponent.urDataFailed(error)
		}

		Component.onQrData: function(data) {
			urScanComponent.qrData(data)
		}

		Component.onUrCaptureStarted: { urScanComponent.urCaptureStarted() }
		Component.onQrCaptureStarted: { urScanComponent.qrCaptureStarted() }

		Component.onPermissionError: function(error) {
			urScanComponent.permissionError(error)
		}

		Component.onCameraSwitched: function(index) {
			urScanComponent.cameraSwitched(index)
		}

		Component.onScannedFrames: function(count, total) {
			urScanComponent.scannedFrames(count, total)
		}

		Component.onEstimatedCompletedPercentage: function(percent) {
			urScanComponent.estimatedCompletedPercentage(percent)
		}
    }
}
