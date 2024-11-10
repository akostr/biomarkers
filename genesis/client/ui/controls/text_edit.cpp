#include "text_edit.h"

#include <genesis_style/style.h>

#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>

////////////////////////////////////////////////////
/// Text edit
TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
}

TextEdit::~TextEdit()
{
}

//// Completer
void TextEdit::SetCompleter(QCompleter *completer)
{
  if (Completer)
    Completer->disconnect(this);

  Completer = completer;

  if (!Completer)
    return;

  Completer->setWidget(this);
  Completer->setCompletionMode(QCompleter::PopupCompletion);
  Completer->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(Completer, QOverload<const QString &>::of(&QCompleter::activated), this, &TextEdit::insertCompletion);
}

QCompleter* TextEdit::GetCompleter() const
{
  return Completer;
}

void TextEdit::HandleInput()
{
}

void TextEdit::insertCompletion(const QString &completion)
{
  if (Completer->widget() != this)
      return;

  QTextCursor tc = textCursor();
  int extra = completion.length() - Completer->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);

  HandleInput();
}

QString TextEdit::textUnderCursor() const
{
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
  if (Completer)
    Completer->setWidget(this);

  QTextEdit::focusInEvent(e);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
  if (Completer && Completer->popup() && Completer->popup()->isVisible())
  {
    // The following keys are forwarded by the completer to the widget
    switch (e->key()) 
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      e->ignore();
      return; // let the completer do default behavior
    default:
      break;
    }
  }

  const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
  if (!Completer || !isShortcut) // do not process the shortcut when we have a completer
  {
    QTextEdit::keyPressEvent(e);
  }

  const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                           e->modifiers().testFlag(Qt::ShiftModifier);
  if (!Completer || (ctrlOrShift && e->text().isEmpty()))
    return;

  static QString eow("~!#$%^&*()+{}|:\"<>?,/;'[]\\-="); // end of word
  const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut 
    && (hasModifier 
     || e->text().isEmpty() 
     || completionPrefix.length() < 1
     || eow.contains(e->text().right(1)))) 
  {
    if (Completer->popup())
      Completer->popup()->hide();

    HandleInput();
    return;
  }

  if (completionPrefix != Completer->completionPrefix()) 
  {
    Completer->setCompletionPrefix(completionPrefix);
    Completer->popup()->setCurrentIndex(Completer->completionModel()->index(0, 0));
  }

  //// Position
  QRect cr = cursorRect();
  int padding = Style::GetSASSValue("inputPaddingScalable").toInt();
  cr.translate(padding, padding);

  if (Completer->popup())
  {
    cr.setWidth(Completer->popup()->sizeHintForColumn(0)
              + Completer->popup()->verticalScrollBar()->sizeHint().width());
  }
  Completer->complete(cr);
}
