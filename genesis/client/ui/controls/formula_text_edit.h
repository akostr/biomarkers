#ifndef FORMULATEXTEDIT_H
#define FORMULATEXTEDIT_H

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QCompleter;
class QMutex;
QT_END_NAMESPACE

class QRegularExpression;

class FormulaTextEdit : public QTextEdit
{
  Q_OBJECT
public:
  struct KeyString;
  FormulaTextEdit(QWidget* parent = nullptr);
  ~FormulaTextEdit();

  void setKeywordsTooltips(const QMap<QString, QPair<QString, int>>& keywordsData);

  void setCompleter(QCompleter *c);
  QCompleter *completer() const;

protected:
  void keyPressEvent(QKeyEvent *e) override;
  void focusInEvent(QFocusEvent *e) override;

private slots:
  void insertCompletion(const QString &completion);

private:
  QString textUnderCursor() const;

private:
  QCompleter *mCompleter = nullptr;

  // QObject interface
public:
  bool event(QEvent *event) override;

protected:
  QString getToolTip(int pos);
  void textPostProcess();

private:
  struct KeywordRange;
  QMap<QString, QPair<QString, int> > mKeywordsData;
  QRegularExpression *mRegEx;
  QList<KeywordRange> mToolTipsList;
  QMutex *mTextMutex;

  struct KeywordRange
  {
    int first;
    int last;
    QString tooltip;
    int id;
    bool has(int pos){return first <= pos && pos <= last;}
    bool less(const KeywordRange& other) const {return first < other.first;}
    static bool compare(const KeywordRange& a, const KeywordRange& b) {return a.less(b);}
  };

};

#endif // FORMULATEXTEDIT_H
