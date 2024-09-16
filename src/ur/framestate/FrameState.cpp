#include "FrameState.h"
#include "qcoreevent.h"

#include <QPainter>
#include <algorithm>
#include <chrono>

FrameState::FrameState(QWidget *targetWidget)
	: QObject(targetWidget)
	, m_targetWidget(targetWidget)
	, m_state(Idle)
	, m_progress(0)
	, m_currentUrFrame(0)
	, m_totalUrFrames(0)
	, m_borderSize(4)
	, m_animationProgress(0)
	, m_estimatedMilliseconds(0)
	, m_processColor(Qt::blue)
	, m_progressColor(Qt::green)
	, m_unscannedUrColor(Qt::lightGray)
	, m_scannedUrColor(Qt::darkGreen)
{
	m_targetWidget->installEventFilter(this);
	connect(&m_animationTimer, &QTimer::timeout, this, &FrameState::animateProcessing);
}

FrameState::~FrameState()
{
	m_targetWidget->removeEventFilter(this);
}

void FrameState::setState(State state)
{
	if (m_state == state)
		return;
	m_state = state;
	if (state == Processing) {
		m_animationTimer.start(50);
	} else {
		m_animationTimer.stop();
	}
	emit stateChanged(m_state);
	updateWidget();
}

void FrameState::setProgress(int progress)
{
	if (m_progress != progress) {
		m_progress = progress;
		updateWidget();
	}
}

void FrameState::setUrFrameInfo(int currentFrame, int totalFrames)
{
	if (m_currentUrFrame != currentFrame || m_totalUrFrames != totalFrames) {
		m_currentUrFrame = currentFrame;
		m_totalUrFrames = totalFrames;
		updateWidget();
	}
}

void FrameState::setProcessColor(const QColor &color) { m_processColor = color; }
void FrameState::setProgressColor(const QColor &color) { m_progressColor = color; }
void FrameState::setUnscannedUrColor(const QColor &color) { m_unscannedUrColor = color; }
void FrameState::setScannedUrColor(const QColor &color) { m_scannedUrColor = color; }
void FrameState::setBorderSize(int size) { m_borderSize = size; }

bool FrameState::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_targetWidget && event->type() == QEvent::Paint)
	{
		QPainter painter(m_targetWidget);
		paint(painter);
		return true;  // Event handled
	}
	return QObject::eventFilter(watched, event);
}

void FrameState::paint(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);

    QRect frameRect = m_targetWidget->rect().adjusted(m_borderSize/2, m_borderSize/2, -m_borderSize/2, -m_borderSize/2);

    QPen pen(Qt::black, m_borderSize);
    painter.setPen(pen);

    switch (m_state) {
    case FrameState::Idle:
        // Don't draw anything in idle state
        return;
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

void FrameState::drawProcessingAnimation(QPainter &painter, const QRect &rect)
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

void FrameState::drawProgressAnimation(QPainter &painter, const QRect &rect)
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

void FrameState::animateProcessing()
{
	m_animationProgress++;
	updateWidget();
}

void FrameState::updateWidget()
{
	if (m_targetWidget) {
        m_targetWidget->update();
	}
}

void FrameState::drawUrFramesProgress(QPainter &painter, const QRect &rect)
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

QPoint FrameState::getPointFromPixel(const QRect &rect, int pixel) const
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

int FrameState::calculateTotalPixels(const QRect &rect) const {
    return (rect.height() * 2 + rect.width() * 2) / m_borderSize - 4;
}

void FrameState::onIdle() {
    updateState(FrameState::Idle);
}

void FrameState::onRecognized() {
    updateState(FrameState::Recognized);
}

void FrameState::onValidated() {
    updateState(FrameState::Validated);
}

void FrameState::onProcessingTimeEstimate(int estimatedMicroSeconds) {
    m_estimatedMilliseconds = estimatedMicroSeconds;
}

void FrameState::onProcessing(int estimatedMicroSeconds) {
    if(m_state != FrameState::Processing) {
        updateState(FrameState::Processing);
        m_estimatedMilliseconds = estimatedMicroSeconds;
    } else {
        m_estimatedMilliseconds += estimatedMicroSeconds;
    }
}

void FrameState::onProgress(int percent) {
    if(m_state != FrameState::Progress)
        updateState(FrameState::Progress);
    m_progress = percent;
}

void FrameState::onError() {
    updateState(FrameState::Error);
}

void FrameState::updateState(State state)
{
    if(state == m_state)
        return; //nothing to do.
    m_state = state;
    if (state == FrameState::Processing) {
        m_animationProgress = 0;
        m_animationTimer.start(50);
    } else {
        m_animationTimer.stop();
    }
    updateWidget();
}

void FrameState::onCurrentFrameUpdate(int currentFrame) {
    m_currentUrFrame = currentFrame;
    updateWidget();
}

void FrameState::onTotalFramesUpdate(int totalFrames) {
    m_totalUrFrames = totalFrames;
    updateWidget();
}

int64_t FrameState::getCurrentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
