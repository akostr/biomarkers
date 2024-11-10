#include "plain_scroll_area.h"

#include <QScrollBar>

PlainScrollArea::PlainScrollArea(QWidget* parent)
  : QScrollArea(parent)
{
}

QSize PlainScrollArea::sizeHint() const
{
  int f = 2 * frameWidth();
  QSize sz(f, f);
  QWidget* w = widget();
  if (w)
  {
    QSize ws = widgetResizable() ? w->sizeHint() : w->size();
    sz += ws;
  }
  sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width() + 5);
  return sz;
}
