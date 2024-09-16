#ifndef FRAMESTATE_H
#define FRAMESTATE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QTimer>

class FrameState : public QObject
{
    Q_OBJECT

public:
    enum State {
        Idle,
        Recognized,
        Validated,
        Processing,
        Progress,
        Error
    };
    Q_ENUM(State)

    explicit FrameState(QWidget *targetWidget);
    ~FrameState();

    void setState(State state);
    void setProgress(int progress);
    void setUrFrameInfo(int currentFrame, int totalFrames);
    void setProcessColor(const QColor &color);
    void setProgressColor(const QColor &color);
    void setUnscannedUrColor(const QColor &color);
    void setScannedUrColor(const QColor &color);
    void setBorderSize(int size);

signals:
    void stateChanged(State state);

private:
    QWidget *m_targetWidget;
    State m_state;
    int m_progress;
    int m_currentUrFrame;
    int m_totalUrFrames;
    int m_borderSize;
    int m_animationProgress;
    int m_estimatedMilliseconds;

    QColor m_processColor;
    QColor m_progressColor;
    QColor m_unscannedUrColor;
    QColor m_scannedUrColor;

    QTimer m_animationTimer;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void paint(QPainter &painter);

    void drawProcessingAnimation(QPainter &painter, const QRect &rect);
    void drawProgressAnimation(QPainter &painter, const QRect &rect);
    void drawUrFramesProgress(QPainter &painter, const QRect &rect);
    QPoint getPointFromPixel(const QRect &rect, int pixel) const;
    int calculateTotalPixels(const QRect &rect) const;
    int64_t getCurrentMilliseconds();

public slots:
    void onIdle();
    void onRecognized();
    void onValidated();
    void onProcessingTimeEstimate(int estimatedMilliseconds = 5000);
    void onProcessing(int estimatedMilliseconds = 5000);
    void onProgress(int percent);
    void onError();
    void onCurrentFrameUpdate(int currentFrame);
    void onTotalFramesUpdate(int totalFrames);
    void onFramesUpdate(int currentFrame, int totalFrames);

private slots:
    void animateProcessing();
    void updateWidget();
    void updateState(State state);
};

#endif // FRAMESTATE_H
