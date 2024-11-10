#include "substring_highlighter.h"
#include "genesis_style/style.h"

SubstringHighlighter::SubstringHighlighter(QTextDocument *parent,
                                           const QStringList& highlightedStrings)
  : QSyntaxHighlighter{parent}
{
  HighlightingRule rule;

  substringFormat.setForeground(Style::GetSASSColor("brandColor"));
  //substringFormat.setFontWeight(QFont::Bold);
  //substringFormat.setFontItalic(true);

  auto strings = highlightedStrings;
  for (auto &str : strings)
    str = str + "\\B";

  rule.pattern = QRegularExpression(strings.join("|"));
  rule.format = substringFormat;
  highlightingRules.append(rule);
}

void SubstringHighlighter::highlightBlock(const QString &text)
{
  for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
