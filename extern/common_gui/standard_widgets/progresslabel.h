#ifndef PROGRESSLABEL_H
#define PROGRESSLABEL_H

#include <QWidget>

class ProgressIcon;
class QLabel;
enum class ProgressIconState;

class ProgressLabel : public QWidget
{
    Q_OBJECT

    ProgressIcon* icon();
    const ProgressIcon* icon() const;

    void init(QLabel* const);
public:
    explicit ProgressLabel(QWidget *parent = nullptr);
    explicit ProgressLabel(const QString& text, QWidget *parent = nullptr);

    void setIconState(const ProgressIconState state);

    QLabel* label();
    const QLabel* label() const;
signals:

};

#endif // PROGRESSLABEL_H
