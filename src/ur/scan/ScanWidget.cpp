// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "ScanWidget.h"
#include "ui_ScanWidget.h"

#include <QComboBox>
#include <QPainter>
#include <QDebug>
#include <algorithm>
#include <chrono>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QMediaDevices>
#include <QPermission>
#else
#include <QCameraInfo>
#include <QCameraExposure>
#endif


ScanWidget::ScanWidget(QWidget *parent, bool manualExposure, int exposureTime)
    : QWidget(parent)
    , ui(new Ui::ScanWidget)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    , m_sink(new QVideoSink(this))
#endif
    , m_thread(new QrScanThread(this))
    , m_frameState(FrameState::Idle)
    , m_animationProgress(0)
{
    ui->setupUi(this);
    
    ui->verticalLayout->setContentsMargins(m_framePadding, m_framePadding, m_framePadding, m_framePadding);

    this->setWindowTitle("Scan QR code");
    
    ui->frame_error->hide();
    ui->frame_error->setInfo(QIcon(":/icons/icons/warning.png"), "Lost connection to camera");

    connect(&m_animationTimer, &QTimer::timeout, this, &ScanWidget::animateProcessing);

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

int64_t getCurrentMilliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void ScanWidget::drawProcessingAnimation(QPainter &painter, const QRect &rect)
{
    static int64_t lastRunTime = getCurrentMilliseconds();
    int64_t currentTime = getCurrentMilliseconds();
    int64_t elapsedMilliseconds = currentTime - lastRunTime;
    lastRunTime = currentTime;

    // Subtract elapsed time from m_estimatedMilliseconds
    m_estimatedMilliseconds -= elapsedMilliseconds;
    if (m_estimatedMilliseconds < 0)
        m_estimatedMilliseconds = 0;

    int totalLength = ((rect.width() + rect.height()) * 2) / m_borderSize - 4;
    m_animationProgress = m_animationProgress % totalLength;
    int currentLength = std::max(std::min((totalLength - 10), m_estimatedMilliseconds / 100), 1);
    if (currentLength <= 0)
        return;  // Animation completed, nothing to draw
    int currentStart = totalLength - m_animationProgress;
    int currentEnd = (currentStart + currentLength) % totalLength;
    int sides[] = {
        (rect.width() - m_borderSize) / m_borderSize,
        (rect.height() - m_borderSize) / m_borderSize,
        (rect.width() - m_borderSize) / m_borderSize,
        (rect.height() - m_borderSize) / m_borderSize
    };
    int starts[] = { 0, sides[0], sides[0] + sides[1], sides[0] + sides[1] + sides[2] };
    int ends[] = { sides[0] - 1, sides[0] + sides[1] - 1, sides[0] + sides[1] + sides[2] - 1, totalLength - 1};

    for (int i = 0; i < 4; i++) {
        int start = starts[i];
        int end = ends[i];
        bool reverse = i > 1;
        bool isHorizontal = (i % 2 == 0);

        auto drawSegment = [&](int segStart, int segEnd) {
            QPoint s, e;
            if (isHorizontal) {
                s = QPoint(!reverse ? (rect.left() + (segStart - start) * m_borderSize) : (rect.right() - (segStart - start) * m_borderSize),
                           !reverse ? rect.top() : (rect.bottom() - m_borderSize));
                e = QPoint(!reverse ? (rect.left() + (segEnd - start + 1) * m_borderSize) : (rect.right() - (segEnd - start + 1) * m_borderSize),
                           !reverse ? (rect.top() + m_borderSize) : rect.bottom());
            } else {
                s = QPoint(!reverse ? (rect.right() - m_borderSize) : rect.left(),
                           !reverse ? (rect.top() + (segStart - start) * m_borderSize) : (rect.bottom() - (segStart - start) * m_borderSize));
                e = QPoint(!reverse ? rect.right() : (rect.left() + m_borderSize),
                           !reverse ? (rect.top() + (segEnd - start + 1) * m_borderSize) : (rect.bottom() - (segEnd - start + 1) * m_borderSize));
            }
            //painter.drawRect(QRect(s, e));
            painter.fillRect(QRect(s,e), m_processColor);
        };

        if (currentStart <= currentEnd) {
            if (currentStart <= start && currentEnd >= end) {
                drawSegment(start, end);
            } else if (start <= currentStart && currentStart <= end) {
                drawSegment(currentStart, std::min(currentEnd, end));
            } else if (start <= currentEnd && currentEnd <= end) {
                drawSegment(start, currentEnd);
            }
        } else {
            if (currentStart <= start || currentEnd >= end) {
                drawSegment(start, end);
            } else {
                if (start <= currentStart && currentStart <= end) {
                    drawSegment(currentStart, end);
                }
                if (start <= currentEnd && currentEnd <= end) {
                    drawSegment(start, currentEnd);
                }
            }
        }
    }
}

void ScanWidget::animateProcessing()
{
    m_animationProgress++;
    update();
}

void ScanWidget::onFrameStateIdle() {
    updateFrameState(FrameState::Idle);
}

void ScanWidget::onFrameStateRecognized() {
    updateFrameState(FrameState::Recognized);
}

void ScanWidget::onFrameStateValidated() {
    updateFrameState(FrameState::Validated);
}

void ScanWidget::onProgressUpdate(int percent) {
    m_progress = percent;
}

void ScanWidget::onProcessingTimeEstimate(int estimatedMicroSeconds) {
    m_estimatedMilliseconds = estimatedMicroSeconds;
}

void ScanWidget::onFrameStateProcessing(int estimatedMicroSeconds) {
    if(m_frameState != FrameState::Processing) {
        updateFrameState(FrameState::Processing);
        m_estimatedMilliseconds = estimatedMicroSeconds;
    } else {
        m_estimatedMilliseconds += estimatedMicroSeconds;
    }
}

void ScanWidget::onFrameStateProgress(int percent) {
    if(m_frameState != FrameState::Progress)
        updateFrameState(FrameState::Progress);
    m_progress = percent;
}

void ScanWidget::onFrameStateError() {
    updateFrameState(FrameState::Error);
}

void ScanWidget::updateFrameState(FrameState state)
{
    if(state == m_frameState)
        return; //nothing to do.
    m_frameState = state;
    if (state == FrameState::Processing) {
        m_animationProgress = 0;
        m_animationTimer.start(50);
    } else {
        m_animationTimer.stop();
    }
    update();
}

void ScanWidget::setProcessColor(const QColor &color) { m_processColor = color; }
void ScanWidget::setProgressColor(const QColor &color) { m_progressColor = color; }
void ScanWidget::setUnscannedUrColor(const QColor &color) { m_unscannedUrColor = color; }
void ScanWidget::setScannedUrColor(const QColor &color) { m_scannedUrColor = color; }

void ScanWidget::onUrFrame(int currentFrame) {
    m_currentUrFrame = currentFrame;
    update();
}

void ScanWidget::onTotalUrFrames(int totalFrames) {
    m_totalUrFrames = totalFrames;
    update();
}

int ScanWidget::calculateTotalPixels(const QRect &rect) const {
    return (rect.height() * 2 + rect.width() * 2) / m_borderSize - 4;
}

void ScanWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect frameRect = rect().adjusted(m_borderSize/2, m_borderSize/2, -m_borderSize/2, -m_borderSize/2);

    QPen pen(Qt::black, m_borderSize);
    painter.setPen(pen);

    switch (m_frameState) {
    case FrameState::Idle:
        // Don't draw anything in idle state
        break;
    case FrameState::Recognized:
        painter.setBrush(QColor(255, 255, 0, 100)); // Semi-transparent yellow
        painter.drawRect(frameRect);
        break;
    case FrameState::Validated:
        painter.setBrush(QColor(0, 255, 0, 100)); // Semi-transparent green
        painter.drawRect(frameRect);
        break;
    case FrameState::Processing:
        pen.setColor(m_processColor);
        painter.setPen(pen);
        drawProcessingAnimation(painter, frameRect);
        break;
    case FrameState::Progress:
        pen.setColor(m_progressColor);
        painter.setPen(pen);
        drawProgressAnimation(painter, frameRect);
        break;
    case FrameState::Error:
        painter.setBrush(QColor(255, 0, 0, 100)); // Semi-transparent red
        painter.drawRect(frameRect);
        break;
    }

    if (m_totalUrFrames > 0) {
        drawUrFramesProgress(painter, frameRect);
    }
}

void ScanWidget::drawProgressAnimation(QPainter &painter, const QRect &rect)
{
    int totalPixels = calculateTotalPixels(rect);
    int progressPixels = totalPixels * m_progress / 100;

    QPoint start = rect.topLeft();
    QPoint end = start;

    for (int i = 0; i < progressPixels; ++i) {
        QPoint nextPoint = getPointFromPixel(rect, i + 1);
        painter.drawLine(end, nextPoint);
        end = nextPoint;
    }
}

void ScanWidget::drawUrFramesProgress(QPainter &painter, const QRect &rect)
{
    int frameSize = qMin(rect.width(), rect.height()) / 5; // Adjust as needed
    int spacing = m_borderSize;
    int columns = rect.width() / (frameSize + spacing);
    int rows = (m_totalUrFrames + columns - 1) / columns;

    for (int i = 0; i < m_totalUrFrames; ++i) {
        int row = i / columns;
        int col = i % columns;
        QRect frameRect(rect.left() + col * (frameSize + spacing),
                        rect.top() + row * (frameSize + spacing),
                        frameSize, frameSize);

        if (i < m_currentUrFrame) {
            painter.fillRect(frameRect, m_scannedUrColor);
        } else {
            painter.fillRect(frameRect, m_unscannedUrColor);
        }

        if (i == m_currentUrFrame - 1) {
            // Pulse effect for the last scanned frame
            int pulseSize = 5; // Adjust as needed
            QRect pulseRect = frameRect.adjusted(-pulseSize, -pulseSize, pulseSize, pulseSize);
            painter.setPen(QPen(m_scannedUrColor, 2));
            painter.drawRect(pulseRect);
        }
    }
}

QPoint ScanWidget::getPointFromPixel(const QRect &rect, int pixel) const
{
    int totalPixels = calculateTotalPixels(rect);
    pixel = (pixel + totalPixels) % totalPixels; // Ensure positive value

    if (pixel < rect.width()) {
        return QPoint(rect.left() + pixel * m_borderSize, rect.top());
    }
    pixel -= rect.width();

    if (pixel < rect.height()) {
        return QPoint(rect.right(), rect.top() + pixel * m_borderSize);
    }
    pixel -= rect.height();

    if (pixel < rect.width()) {
        return QPoint(rect.right() - pixel * m_borderSize, rect.bottom());
    }
    pixel -= rect.width();

    return QPoint(rect.left(), rect.bottom() - pixel * m_borderSize);
}

void ScanWidget::startCapture(bool scan_ur) {
    m_scan_ur = scan_ur;
    ui->progressBar_UR->setVisible(m_scan_ur);
    ui->progressBar_UR->setFormat("Progress: %v%");

    updateFrameState(FrameState::Idle);

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

    if (index < 0) {
        return;
    }
    
    if (index >= cameras.size()) {
        return;
    }

    if (m_camera) {
        m_camera->stop();
    }

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
    if (m_done) {
        return;
    }
    
    if (m_scan_ur) {
        bool success = m_decoder.receive_part(data.toStdString());
        if (!success) {
          return;
        }

    updateFrameState(FrameState::Recognized);

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
    if (!m_decoder.is_success()) {
        return "";
    }

    ur::ByteVector cbor = m_decoder.result_ur().cbor();
    std::string data;
    auto i = cbor.begin();
    auto end = cbor.end();
    ur::CborLite::decodeBytes(i, end, data);
    return data;
}

std::string ScanWidget::getURType() {
    if (!m_decoder.is_success()) {
        return "";
    }

    return m_decoder.expected_type().value_or("");
}

QString ScanWidget::getURError() {
    if (!m_decoder.is_failure()) {
        return {};
    }
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
