// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "ScanWidget.h"
#include "ui_ScanWidget.h"

#include <QComboBox>
#include <QDebug>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#include <QPermission>
#else
#include <QCameraInfo>
#include <QCameraExposure>
#endif


ScanWidget::ScanWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScanWidget)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    , m_sink(new QVideoSink(this))
#endif
    , m_thread(new QrScanThread(this))
{
    ui->setupUi(this);
    
    ui->verticalLayout->setContentsMargins(m_framePadding, m_framePadding, m_framePadding, m_framePadding);

    this->setWindowTitle("Scan QR code");
    
    ui->frame_error->hide();
    ui->frame_error->setInfo(QIcon(":/icons/icons/warning.png"), "Lost connection to camera");

    this->refreshCameraList();
    
    connect(ui->combo_camera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScanWidget::onCameraSwitched);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(ui->viewfinder->videoSink(), &QVideoSink::videoFrameChanged, this, &ScanWidget::handleFrameCaptured);
#endif
    connect(ui->btn_refresh, &QPushButton::clicked, [this]{
        this->refreshCameraList();
        this->onCameraSwitched(0);
    });
    connect(m_thread, &QrScanThread::decoded, this, &ScanWidget::onDecoded);

    connect(ui->check_manualExposure, &QCheckBox::toggled, [this](bool enabled) {
        if (!m_camera) {
            return;
        }

        ui->slider_exposure->setVisible(enabled);
        if (enabled) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            m_camera->setExposureMode(QCamera::ExposureManual);
#else
            m_camera->exposure()->setExposureMode(QCameraExposure::ExposureManual);
#endif
        } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            // Qt-bug: this does not work for cameras that only support V4L2_EXPOSURE_APERTURE_PRIORITY
            // Check with v4l2-ctl -L
            m_camera->setExposureMode(QCamera::ExposureAuto);
#else
            m_camera->exposure()->setExposureMode(QCameraExposure::ExposureAuto);
#endif
        }
        // conf()->set(Config::cameraManualExposure, enabled);
        m_manualExposure = enabled;
        emit this->manualExposureEnabledChanged(enabled);
    });

    connect(ui->slider_exposure, &QSlider::valueChanged, [this](int value) {
        if (!m_camera) {
            return;
        }

        float exposure = 0.00033 * value;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_camera->setExposureMode(QCamera::ExposureManual);
        m_camera->setManualExposureTime(exposure);
#else
        if(m_camera->exposure()->exposureMode() != QCameraExposure::ExposureManual)
            m_camera->exposure()->setExposureMode(QCameraExposure::ExposureManual);
        m_camera->exposure()->setManualAperture(exposure);
#endif
        // conf()->set(Config::cameraExposureTime, value);
        m_exposureTime = value;
        emit this->exposureTimeChanged(value);
    });

    ui->check_manualExposure->setVisible(false);
    ui->slider_exposure->setVisible(false);
}


void ScanWidget::startCapture(bool scan_ur) {
    if(scan_ur)
       emit urCaptureStarted();
    else
       emit captureStarted();
    m_scan_ur = scan_ur;
    ui->progressBar_UR->setVisible(m_scan_ur);
    ui->progressBar_UR->setFormat("Progress: %v%");

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
        case Qt::PermissionStatus::Undetermined:
            qDebug() << "Camera permission undetermined";
            qApp->requestPermission(cameraPermission, [this] {
                startCapture(m_scan_ur);
            });
            return;
        case Qt::PermissionStatus::Denied:
	    emit permissionError("No permission to start camera.");
            ui->frame_error->setText("No permission to start camera.");
            ui->frame_error->show();
            qDebug() << "No permission to start camera.";
            return;
        case Qt::PermissionStatus::Granted:
            qDebug() << "Camera permission granted";
            break;
    }
#endif

    if (ui->combo_camera->count() < 1) {
        ui->frame_error->setText("No cameras found. Attach a camera and press 'Refresh'.");
        ui->frame_error->show();
        qDebug() << "No cameras found. Attach a camera and press 'Refresh'.";
        return;
    }
    
    this->onCameraSwitched(0);
    
    if (!m_thread->isRunning()) {
        m_thread->start();
    }
}

void ScanWidget::reset() {
    this->decodedString = "";
    m_done = false;
    ui->progressBar_UR->setValue(0);
    m_decoder = ur::URDecoder();
    m_thread->start();
    m_handleFrames = true;
}

void ScanWidget::stop() {
    m_camera->stop();
    m_thread->stop();
}

void ScanWidget::pause() {
    m_handleFrames = false;
}

void ScanWidget::refreshCameraList() {
    ui->combo_camera->clear();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const auto &camera : cameras) {
        ui->combo_camera->addItem(camera.description());
    }
#else
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for (const auto &cameraInfo : cameras) {
        ui->combo_camera->addItem(cameraInfo.description());
    }
#endif
}

void ScanWidget::handleFrameCaptured(const QVideoFrame &frame) {
    if (!m_handleFrames) {
        return;
    }
    
    if (!m_thread->isRunning()) {
        return;
    }

    QImage img = this->videoFrameToImage(frame);
    if (img.format() == QImage::Format_ARGB32) {
        m_thread->addImage(img);
    }
}

QImage ScanWidget::videoFrameToImage(const QVideoFrame &videoFrame)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImage image = videoFrame.toImage();
#else
    QImage image = videoFrame.image();
#endif

    if (image.isNull()) {
        return {};
    }
    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    return image.copy();
}


void ScanWidget::onCameraSwitched(int index) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
#else
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
#endif

    if (index < 0)
        return;
    if (index >= cameras.size())
        return;
    if (m_camera)
        m_camera->stop();
    ui->frame_error->setVisible(false);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_camera.reset(new QCamera(cameras.at(index), this));
    m_captureSession.setCamera(m_camera.data());
    m_captureSession.setVideoOutput(ui->viewfinder);
    bool manualExposureSupported = m_camera->isExposureModeSupported(QCamera::ExposureManual);
#else
    m_camera.reset(new QCamera(cameras.at(index), this));
    m_viewfinder.reset(new QCameraViewfinder());
    m_camera->setViewfinder(m_viewfinder.data());
    ui->viewfinder->setLayout(new QVBoxLayout());
    ui->viewfinder->layout()->addWidget(m_viewfinder.data());
    bool manualExposureSupported = m_camera->exposure()->isExposureModeSupported(QCameraExposure::ExposureManual);

    if(m_probe) {
        m_probe->setSource(static_cast<QMediaObject*>(nullptr));
        disconnect(m_probe.data(), &QVideoProbe::videoFrameProbed, this, &ScanWidget::handleFrameCaptured);
    }
    m_probe.reset(new QVideoProbe(this));
    if (m_probe->setSource(static_cast<QMediaObject*>(m_camera.data()))) {
        connect(m_probe.data(), &QVideoProbe::videoFrameProbed, this, &ScanWidget::handleFrameCaptured);
    } else {
        qWarning() << "Failed to set probe source";
    }
#endif

    ui->check_manualExposure->setVisible(manualExposureSupported);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    qDebug() << "Supported camera features: " << m_camera->supportedFeatures();
    qDebug() << "Current focus mode: " << m_camera->focusMode();
    if (m_camera->isExposureModeSupported(QCamera::ExposureBarcode)) {
        qDebug() << "Barcode exposure mode is supported";
    }

    connect(m_camera.data(), &QCamera::activeChanged, [this](bool active){
        ui->frame_error->setText("Lost connection to camera");
        ui->frame_error->setVisible(!active);
        if (!active)
                qDebug() << "Lost connection to camera";
    });
    connect(m_camera.data(), &QCamera::errorOccurred, [this](QCamera::Error error, const QString &errorString) {
        if (error == QCamera::Error::CameraError) {
            ui->frame_error->setText(QString("Error: %1").arg(errorString));
            ui->frame_error->setVisible(true);
            qDebug() << QString("Error: %1").arg(errorString);
        }
    });
#else
    if (m_camera->exposure()->isExposureModeSupported(QCameraExposure::ExposureBarcode)) {
        qDebug() << "Barcode exposure mode is supported";
    }
    connect(m_camera.data(), &QCamera::statusChanged, [this](QCamera::Status status){
        bool active = (status == QCamera::ActiveStatus);
        ui->frame_error->setText("Lost connection to camera");
        ui->frame_error->setVisible(!active);
        if (!active)
            qDebug() << "Lost connection to camera";
    });
#endif
    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), [this](QCamera::Error error) {
        if (error != QCamera::Error::CameraError)
            return;
        ui->frame_error->setText(QString("Error: %1").arg(m_camera->errorString()));
        ui->frame_error->setVisible(true);
        qDebug() << QString("Error: %1").arg(m_camera->errorString());
    });

    m_camera->start();
    emit cameraSwitched(index);

    // bool useManualExposure = conf()->get(Config::cameraManualExposure).toBool() && manualExposureSupported;
    // ui->check_manualExposure->setChecked(useManualExposure);
    // if (useManualExposure) {
    //    ui->slider_exposure->setValue(conf()->get(Config::cameraExposureTime).toInt());
    //}
    ui->check_manualExposure->setChecked(m_manualExposure);
    if(m_manualExposure)
        ui->slider_exposure->setValue(m_exposureTime);
}

void ScanWidget::onDecoded(const QString &data) {
    if (m_done)
        return;
    if (m_scan_ur) {
        bool success = m_decoder.receive_part(data.toStdString());
        if (!success)
            return;
        // updateFrameState(FrameState::Recognized); // TODO
        ui->progressBar_UR->setValue(m_decoder.estimated_percent_complete() * 100);
        ui->progressBar_UR->setMaximum(100);
        if (m_decoder.is_complete()) {
            m_done = true;
            m_thread->stop();
            emit finished(m_decoder.is_success());
        }
        return;
    }
    decodedString = data;
    m_done = true;
    m_thread->stop();
    emit finished(true);
}

std::string ScanWidget::getURData() {
    if (!m_decoder.is_success())
        return "";
    ur::ByteVector cbor = m_decoder.result_ur().cbor();
    std::string data;
    auto i = cbor.begin();
    auto end = cbor.end();
    ur::CborLite::decodeBytes(i, end, data);
    return data;
}

std::string ScanWidget::getURType() {
    if (!m_decoder.is_success())
        return "";
    return m_decoder.expected_type().value_or("");
}

QString ScanWidget::getURError() {
    if (!m_decoder.is_failure())
        return {};
    return QString::fromStdString(m_decoder.result_error().what());
}

ScanWidget::~ScanWidget()
{
    m_thread->stop();
    m_thread->quit();
    if (!m_thread->wait(5000))
    {
        m_thread->terminate();
        m_thread->wait();
    }
}
