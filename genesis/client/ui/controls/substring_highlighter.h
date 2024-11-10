#ifndef SUBSTRINGHIGHLIGHTER_H
#define SUBSTRINGHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class SubstringHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
public:
  explicit SubstringHighlighter(QTextDocument *parent = nullptr,
                                const QStringList &highlightedStrings = {});

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule
  {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QList<HighlightingRule> highlightingRules;

  QTextCharFormat substringFormat;
};

#endif // SUBSTRINGHIGHLIGHTER_H
