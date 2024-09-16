import QtQuick 2.0
import UrOts 0.1

Item {
    id: qrDisplayComponent

    width: 200
    height: 200

	function setQrCode(data) {
		qrWidget.setQrCode(data)
	}

    QrWidget {
        id: qrWidget
        anchors.fill: parent
    }
}
