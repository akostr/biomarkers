#include "formula_text_edit.h"
#include <QHelpEvent>
#include <QToolTip>
#include <QRegularExpression>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QMutex>
#include <QString>

const QChar kParagraphChar(8233);

FormulaTextEdit::FormulaTextEdit(QWidget *parent)
  :QTextEdit(parent),
  mRegEx(new QRegularExpression),
  mTextMutex(new QMutex())
{
  setText("@Chemstat HTSA #22-20M#6 + @K30B#9 - @Chemstat HTSA #22-20M#6 -\u2028@DCPPP#3 / @PDCB#5 * @DCPPP#3\n\
+ @Chemstat HTSA #22-20M#6 * @PDCB#5 - @DCP#2");
  connect(this, &QTextEdit::textChanged, this, &FormulaTextEdit::textPostProcess);
  textPostProcess();
}

FormulaTextEdit::~FormulaTextEdit()
{
  delete mRegEx;
}

void FormulaTextEdit::setKeywordsTooltips(const QMap<QString, QPair<QString, int> > &keywordsData)
{
  mKeywordsData = keywordsData;
  QStringList keywords = keywordsData.keys();
  for(auto& keyword : keywords)
  {
    keyword.append("\\B");//regexp tag for proper work
  }
  mRegEx->setPattern(keywords.join("|"));
}

void FormulaTextEdit::setCompleter(QCompleter *completer)
{
  if (mCompleter)
    mCompleter->disconnect(this);

  mCompleter = completer;

  if (!mCompleter)
    return;

  mCompleter->setWidget(this);
  mCompleter->setCompletionMode(QCompleter::PopupCompletion);
  mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(mCompleter, QOverload<const QString &>::of(&QCompleter::activated),
                   this, &FormulaTextEdit::insertCompletion);
}

QCompleter *FormulaTextEdit::completer() const
{
  return mCompleter;
}

void FormulaTextEdit::keyPressEvent(QKeyEvent *e)
{
  if(!mCompleter->popup())
    return QTextEdit::keyPressEvent(e);
  if (mCompleter && mCompleter->popup()->isVisible()) {
    // The following keys are forwarded by the completer to the widget
    switch (e->key()) {
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

  const bool isShortcut = (e->modifiers().testFlag(Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+E
  if (!mCompleter || !isShortcut) // do not process the shortcut when we have a completer
    QTextEdit::keyPressEvent(e);
  //! [7]

  //! [8]
  const bool ctrlOrShift = e->modifiers().testFlag(Qt::ControlModifier) ||
                           e->modifiers().testFlag(Qt::ShiftModifier);
  if (!mCompleter || (ctrlOrShift && e->text().isEmpty()))
    return;

  static QString eow("~!$%^&*() _+{}|:\"<>?,./;'\\-="); // end of word
  const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();
  if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
                      || eow.contains(e->text().right(1)))) {
    mCompleter->popup()->hide();
    return;
  }

  if (completionPrefix != mCompleter->completionPrefix()) {
    mCompleter->setCompletionPrefix(completionPrefix);
    mCompleter->popup()->setCurrentIndex(mCompleter->completionModel()->index(0, 0));
  }
  QRect cr = cursorRect();
  cr.setWidth(mCompleter->popup()->sizeHintForColumn(0)
              + mCompleter->popup()->verticalScrollBar()->sizeHint().width());
  mCompleter->complete(cr); // popup it up!
}

void FormulaTextEdit::focusInEvent(QFocusEvent *e)
{
  if (mCompleter)
    mCompleter->setWidget(this);
  QTextEdit::focusInEvent(e);
}

void FormulaTextEdit::insertCompletion(const QString &completion)
{
  if (mCompleter->widget() != this)
    return;
  QTextCursor tc = textCursor();
  tc.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, mCompleter->completionPrefix().length());
  tc.insertText(completion);
  setTextCursor(tc);
}

QString FormulaTextEdit::textUnderCursor() const
{
  QTextCursor tc = textCursor();
  int charIndex = tc.positionInBlock() - 1;
  tc.select(QTextCursor::BlockUnderCursor);
  QString selText = tc.selectedText();
  if(selText.startsWith(kParagraphChar))
  {
    selText.removeFirst();
  }
  if(charIndex < 0)
    return QString();
  if(selText.isEmpty())
    return selText;
  // qDebug() << charIndex << QString("(%1)").arg(selText[charIndex]);
  // qDebug() << selText;
  if(selText.isEmpty())
    return QString();
  int atSignIndex = -1;
  for(int c = charIndex; c >= 0; c--)
  {
    if(selText[c] == '@')
    {
      atSignIndex = c;
      break;
    }
  }
  // qDebug() << atSignIndex;
  if(atSignIndex == -1)
    return QString();
  if(charIndex < selText.size() - 1)
    selText.remove(charIndex + 1, selText.size() - (charIndex + 1));
  // qDebug() << selText;
  if(atSignIndex > 0)
    selText.remove(0, atSignIndex);
  // qDebug() << selText;
  return selText;
}

void FormulaTextEdit::textPostProcess()
{
  mToolTipsList.clear();
  auto matchIterator = mRegEx->globalMatch(toPlainText());
  while(matchIterator.hasNext())
  {
    auto match = matchIterator.next();
    auto keywordData = mKeywordsData.value(match.captured());
    if(keywordData.first.isEmpty() && keywordData.second == 0)
      continue;
    KeywordRange range;
    range.tooltip = keywordData.first;
    range.id = keywordData.second;
    range.first = match.capturedStart(0);
    range.last = match.capturedEnd(0);
    mToolTipsList << range;
  }
  std::sort(mToolTipsList.begin(), mToolTipsList.end(), &KeywordRange::compare);
}

bool FormulaTextEdit::event(QEvent *event)
{
  if (event->type() == QEvent::ToolTip)
  {
    QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
    QTextCursor cursor = cursorForPosition(helpEvent->pos());
    auto toolTip = getToolTip(cursor.position());

    if (!toolTip.isEmpty())
      QToolTip::showText(helpEvent->globalPos(), toolTip, this);
    else
      QToolTip::hideText();
    return true;
  }
  return QTextEdit::event(event);
}

QString FormulaTextEdit::getToolTip(int pos)
{
  if(mToolTipsList.isEmpty())
    return QString();

  auto comp = [](const KeywordRange& range, int value)->bool{return range.first < value;};
  auto first = std::lower_bound(mToolTipsList.begin(), mToolTipsList.end(), pos, comp);
  auto actual = first - 1;
  if(actual == mToolTipsList.end())
    return QString();
  if(actual->first <= pos && actual->last > pos)
    return actual->tooltip;
  else
    return QString();
}
