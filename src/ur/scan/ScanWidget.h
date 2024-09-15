// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QScopedPointer>
#include <QTimer>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QCameraViewfinder>
#include <QVideoProbe>
#include <QVideoFrame>
#else
#include <QMediaCaptureSession>
#include <QVideoSink>
#endif

#include "QrScanThread.h"

#include <../bcur/bc-ur.hpp>
#include <../bcur/ur-decoder.hpp>

namespace Ui {
    class ScanWidget;
}

class ScanWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit ScanWidget(QWidget *parent);
    ~ScanWidget() override;

    QString decodedString = "";
    std::string getURData();
    std::string getURType();
    QString getURError();
    
    void startCapture(bool scan_ur = false);
    void reset();
    void stop();
    void pause();

signals:
    void finished(bool success);
    void manualExposureEnabledChanged(bool enabled);
    void exposureTimeChanged(int value);
    void setManualExposure(bool enabled);
    void urCaptureStarted();
    void captureStarted();
    void permissionError(const QString &text);
    void cameraSwitched(int index);
    
private slots:
    void onCameraSwitched(int index);
    void onDecoded(const QString &data);

private:
    void refreshCameraList();
    QImage videoFrameToImage(const QVideoFrame &videoFrame);
    void handleFrameCaptured(const QVideoFrame &videoFrame);

    QScopedPointer<Ui::ScanWidget> ui;

    bool m_scan_ur = false;
    QrScanThread *m_thread;
    QScopedPointer<QCamera> m_camera;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QScopedPointer<QCameraViewfinder> m_viewfinder;
    QScopedPointer<QVideoProbe> m_probe;
#else
    QMediaCaptureSession m_captureSession;
    QVideoSink m_sink;
#endif
    ur::URDecoder m_decoder;
    bool m_done = false;
    bool m_handleFrames = true;
    bool m_manualExposure = false;
    int m_exposureTime = 10;

    int m_framePadding = 14;
};

#endif //SCANWIDGET_H
