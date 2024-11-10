#include "progressicon.h"

#include <QTimer>
#include <QPainter>
#include <QTime>

namespace {

const int FPS = 60;
static const int PHASES_COUNT = 60;
static const qreal ROTATION_FREQUENCY = .5;

}

const QVector<QPixmap>& ProgressIcon::initOk()
{
    static const QVector<QPixmap> PHASES{([](){
        const QPixmap baseFrame(":/resource/progress.svg");
        Q_ASSERT(!baseFrame.isNull());
        Q_ASSERT(baseFrame.width() == baseFrame.height());
        QVector<QPixmap> result(PHASES_COUNT, baseFrame);
        for (int frameId = 1; frameId < result.size(); ++frameId) {
            QPixmap frame(baseFrame.size());
            frame.fill(Qt::transparent);
            QPainter painter(&frame);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.translate(frame.width() / 2, frame.height() / 2);
            const double angle = 360. / PHASES_COUNT * frameId;
            painter.rotate(angle);
            painter.drawPixmap(-frame.width() / 2, -frame.height() / 2, baseFrame);
            result[frameId] = frame;
        }
        return result;
    })()};

    return PHASES;
}

const QVector<QPixmap>& ProgressIcon::initWarning()
{
    static const QVector<QPixmap> PHASES{([](){
        const QPixmap baseFrame(":/resource/progress_warning.svg");
        const QPixmap staticFrame(":/resource/progress_warning_static.svg");
        Q_ASSERT(!baseFrame.isNull());
        Q_ASSERT(baseFrame.width() == baseFrame.height());
        QVector<QPixmap> result(PHASES_COUNT, baseFrame);
        for (int frameId = 0; frameId < result.size(); ++frameId) {
            QPixmap frame(baseFrame.size());
            frame.fill(Qt::transparent);
            QPainter painter(&frame);
            painter.drawPixmap(0, 0, staticFrame);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.translate(frame.width() / 2, frame.height() / 2);
            const double angle = 360. / PHASES_COUNT * frameId;
            painter.rotate(angle);
            painter.drawPixmap(-frame.width() / 2, -frame.height() / 2, baseFrame);
            result[frameId] = frame;
        }
        return result;
    })()};

    return PHASES;
}

const QPixmap& ProgressIcon::initPause() {
    static const QPixmap PAUSE{":/resource/progress_pause.svg"};
    return PAUSE;
}

const QPixmap& ProgressIcon::initDisabled() {
    static const QPixmap DISABLED{[](){
        QImage img{":/resource/progress_pause.svg"};
        const QImage alpha{img.alphaChannel()};

        return QPixmap::fromImage(img.convertToFormat(QImage::Format_ARGB32));
    }()};
    return DISABLED;
}

ProgressIcon::ProgressIcon(QWidget *parent)
    : QWidget{parent}
    , timer(new QTimer(this))
{
    timer->setInterval(1000 / FPS);
    this->setMinimumSize(initOk()[0].size());

    timer->callOnTimeout([this](){ this->update(); });
}

void ProgressIcon::setState(const ProgressIconState newState)
{
    state = newState;
    if (state == ProgressIconState::Ok || state == ProgressIconState::Warning) {
        timer->start();
    } else {
        timer->stop();
        this->update();
    }
}

ProgressIconState ProgressIcon::getState() const
{
    return state;
}

void ProgressIcon::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    const QPixmap* frame{&initPause()};
    if (state == ProgressIconState::Ok || state == ProgressIconState::Warning) {
        const qreal angle = QTime::currentTime().msecsSinceStartOfDay() / 1000. * 360. * ROTATION_FREQUENCY;
        const qreal step = 360 / PHASES_COUNT;
        const int frameId = static_cast<int>(angle / step) % PHASES_COUNT;
        if (state == ProgressIconState::Ok) {
            frame = &initOk()[frameId];
        } else {
            frame = &initWarning()[frameId];
        }
    }
    if (!this->isEnabled()) {
        frame = &initDisabled();
    }
    painter.drawPixmap((this->width() - frame->width()) / 2, (this->height() - frame->height()) / 2, *frame);
}
