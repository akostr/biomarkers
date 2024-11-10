#ifndef PROGRESSICON_H
#define PROGRESSICON_H
#include <QWidget>
#include <QtGlobal>

enum class ProgressIconState {
    Ok,
    Warning,
    Stoped,
};

class ProgressIcon : public QWidget
{
    Q_OBJECT

    QTimer* const timer;
    ProgressIconState state;

    void paintEvent(QPaintEvent *) final;

    static const QVector<QPixmap>& initOk();
    static const QVector<QPixmap>& initWarning();
    static const QPixmap& initPause();
    static const QPixmap& initDisabled();
public:
    explicit ProgressIcon(QWidget *parent = nullptr);

    void setState(const ProgressIconState);
    ProgressIconState getState() const;
};

#endif // PROGRESSICON_H
