#include "elided_label.h"

#include <QPainter>
#include <QTextLayout>
#include <QDebug>

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent)
  : QLabel(text, parent)
  , Elided(false)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

ElidedLabel::ElidedLabel(QWidget* parent)
  : ElidedLabel(QString(), parent)
{
}

ElidedLabel::~ElidedLabel()
{
}

bool ElidedLabel::IsElided() const
{
  return Elided;
}

void ElidedLabel::paintEvent(QPaintEvent *event)
{
  QFrame::paintEvent(event);
  QString content = text();

  QPainter painter(this);
  QFontMetrics fontMetrics = painter.fontMetrics();

  bool didElide = false;
  int lineSpacing = fontMetrics.lineSpacing();
  int y = 0;

  QTextLayout textLayout(content, painter.font());
  textLayout.beginLayout();
  forever 
  {
    QTextLine line = textLayout.createLine();

    if (!line.isValid())
      break;

    line.setLineWidth(width());
    int nextLineY = y + lineSpacing;

    if (height() >= nextLineY + lineSpacing) 
    {
        line.draw(&painter, QPoint(0, y));
        y = nextLineY;
    } 
    else 
    {
        QString lastLine = content.mid(line.textStart());
        QString elidedLastLine = fontMetrics.elidedText(lastLine, Qt::ElideRight, width());
        painter.drawText(QPoint(0, y + fontMetrics.ascent()), elidedLastLine);
        line = textLayout.createLine();
        didElide = line.isValid();
        break;
    }
  }
  textLayout.endLayout();

  if (didElide != Elided) 
  {
    Elided = didElide;
    emit ElisionChanged(didElide);
  }
}
