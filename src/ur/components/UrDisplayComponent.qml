import QtQuick 2.0
import UrOts 0.1

Item {
    id: urDisplayComponent

	width: 200
	height: 200
    
    function setData(type, data) {
        urWidget.setData(type, data)
    }

    UrWidget {
        id: urWidget
        anchors.fill: parent
    }
}
