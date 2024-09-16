#include "UrRegister.h"

#include <QQmlEngine>
#include "../display/UrWidget.h"
#include "../display/QrWidget.h"
#include "../scan/ScanWidget.h"

namespace UrRegister {

	void registerTypes()
	{
		qmlRegisterType<UrWidget>("UrOts", 0, 1, "UrWidget");
		qmlRegisterType<QrWidget>("UrOts", 0, 1, "QrWidget");
		qmlRegisterType<ScanWidget>("UrOts", 0, 1, "ScanWidget");
	}

}
