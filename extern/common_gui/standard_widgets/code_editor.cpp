#include "code_editor.h"

#include <QPainter>
#include <QTextBlock>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
  NumberArea = new LineNumberArea(this);

  connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::UpdateLineNumberAreaWidth);
  connect(this, &CodeEditor::updateRequest, this, &CodeEditor::UpdateLineNumberArea);

  UpdateLineNumberAreaWidth(0);
}

CodeEditor::~CodeEditor()
{
}

int CodeEditor::LineNumberAreaWidth()
{
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10)
  {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 3;

  return space;
}

void CodeEditor::UpdateLineNumberAreaWidth(int /* newBlockCount */)
{
  setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::UpdateLineNumberArea(const QRect &rect, int dy)
{
  if (dy)
    NumberArea->scroll(0, dy);
  else
    NumberArea->update(0, rect.y(), NumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    UpdateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  NumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

void CodeEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
  QPainter painter(NumberArea);
  painter.fillRect(event->rect(), QColor("#eeeeee"));

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom())
  {
    if (block.isVisible() && bottom >= event->rect().top())
    {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(Qt::black);
      painter.setFont(font());
      painter.drawText(0, top, NumberArea->width() - 3, fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

