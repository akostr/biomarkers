#include "progresslabel.h"

#include <QHBoxLayout>
#include <QLabel>

#include "progressicon.h"

void ProgressLabel::init(QLabel* const lbl) {
    QHBoxLayout* const layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    this->setLayout(layout);
    layout->addWidget(new ProgressIcon(this));
    layout->addSpacing(4);
    layout->addWidget(lbl, Qt::AlignLeft);
}

ProgressLabel::ProgressLabel(QWidget *parent)
    : QWidget{parent}
{
    init(new QLabel(this));
}


ProgressLabel::ProgressLabel(const QString& text, QWidget *parent)
    : QWidget{parent}
{
    init(new QLabel(text, this));
}

void ProgressLabel::setIconState(const ProgressIconState state)
{
    this->icon()->setState(state);
}

QLabel* ProgressLabel::label()
{
    QWidget* const w = this->layout()->itemAt(2)->widget();
    Q_ASSERT(qobject_cast<QLabel*>(w) != nullptr);
    return static_cast<QLabel*>(w);
}

const QLabel* ProgressLabel::label() const
{
    QWidget* const w = this->layout()->itemAt(2)->widget();
    Q_ASSERT(qobject_cast<QLabel*>(w) != nullptr);
    return static_cast<QLabel*>(w);
}

ProgressIcon* ProgressLabel::icon()
{
    QWidget* const w = this->layout()->itemAt(0)->widget();
    Q_ASSERT(qobject_cast<ProgressIcon*>(w) != nullptr);
    return static_cast<ProgressIcon*>(w);
}

const ProgressIcon* ProgressLabel::icon() const
{
    QWidget* const w = this->layout()->itemAt(0)->widget();
    Q_ASSERT(qobject_cast<ProgressIcon*>(w) != nullptr);
    return static_cast<ProgressIcon*>(w);
}
