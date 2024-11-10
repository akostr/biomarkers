#include "text_edit_emails.h"

#include <genesis_style/style.h>

#include <QRegExp>
#include <QPainter>
#include <QTextCursor>

#include <QDebug>

namespace Details
{
  QString EmailRegex = "(?:[a-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-zA-Za-zA-Z0-9!#$%&'*+/=?^_`{|}~-]+)*|\"(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21\\x23-\\x5b\\x5d-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])*\")@(?:(?:[a-zA-Z0-9-](?:[a-zA-Z0-9-]*[a-zA-Z0-9-])?\\.)+[a-zA-Z0-9-](?:[a-zA-Z0-9-]*[a-zA-Z0-9-])?|\\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-zA-Z0-9-]*[a-zA-Z0-9-]:(?:[\\x01-\\x08\\x0b\\x0c\\x0e-\\x1f\\x21-\\x5a\\x53-\\x7f]|\\\\[\\x01-\\x09\\x0b\\x0c\\x0e-\\x7f])+)\\])";
}

////////////////////////////////////////////////////
/// Text edit / email list
TextEditEmails::TextEditEmails(QWidget* parent)
  : TextEdit(parent)
{
}

TextEditEmails::~TextEditEmails()
{
}

//// Set Display Names
void TextEditEmails::SetDisplayNames(const QMap<QString/*email*/, QString/*dispalyname*/>& displayNames)
{
  DisplayNames = displayNames;
  for (auto dn = displayNames.begin(); dn != displayNames.end(); dn++)
  {
    DisplayNames[dn.key().toLower()] = dn.value();
  }
}

//// Set ids
void TextEditEmails::SetIds(const QMap<QString/*email*/, int/*id*/>& ids)
{
  Ids = ids;
  for (auto id = ids.begin(); id != ids.end(); id++)
  {
    Ids[id.key().toLower()] = id.value();
  }
}

QStringList TextEditEmails::GetInputEmails()
{
  QString htmlText = document()->toHtml();
  htmlText.remove(QChar(8203));

  //// Validated emails
  QStringList emails;
  QRegExp rx(Details::EmailRegex);

  //// Collect
  int pos = 0;
  while ((pos = rx.indexIn(htmlText, pos)) != -1)
  {
    emails << htmlText.mid(pos, rx.matchedLength());
    pos += rx.matchedLength();
  }

  return emails;
}

QList<int> TextEditEmails::GetInputIds()
{
  QSet<int> ids;

  auto emails = GetInputEmails();
  for (auto& email : emails)
  {
    if (Ids.contains(email))
    {
      ids << Ids[email.toLower()];
    }
  }

  return QList<int>(ids.begin(), ids.end());
}

void TextEditEmails::HandleInput()
{
  QString htmlText = document()->toHtml();

  //// Validated emails
  QStringList emails;
  QRegExp rx(Details::EmailRegex);

  //// Collect
  int pos = 0;
  while ((pos = rx.indexIn(htmlText, pos)) != -1) 
  {
    emails << htmlText.mid(pos, rx.matchedLength());
    pos += rx.matchedLength();
  }

  //// Format
  {
    for (auto email : emails)
    {
      QString displayName = DisplayNames[email.toLower()];
      if (displayName.isEmpty())
        displayName = email;

      displayName.replace("@", "&#8203;@&#8203;"); //// <- shall prevent from further email regexp matches
      displayName = QString("%1").arg(displayName);

      QString format = Style::ApplySASS(
        "<table cellpadding=\"0px\"><tr><td>"
        "<span style=\"color: @textColorLink; font-size: @fontSizeNormal; font-weight: @fontWeightNormal;\">%1</span>"
        "</td>"
        "</tr></table>&nbsp;"
      );
      QString formatted = format.arg(displayName);
      htmlText.replace(email, formatted);
    }
  }

  //// Apply
  if (!emails.empty())
  {
    setHtml(htmlText);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
  }
}
