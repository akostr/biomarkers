#pragma once

#include "text_edit.h"

////////////////////////////////////////////////////
/// Text edit / email list
class TextEditEmails : public TextEdit
{
  Q_OBJECT

public:
  TextEditEmails(QWidget *parent = nullptr);
  ~TextEditEmails();

public:
  //// Set Display Names
  void SetDisplayNames(const QMap<QString/*email*/, QString/*dispalyname*/>& displayNames);
  
  //// Set ids
  void SetIds(const QMap<QString/*email*/, int/*id*/>& ids);

  //// Get input emails
  QStringList GetInputEmails();

  //// Get input ids
  QList<int> GetInputIds();

  virtual void HandleInput() override;

protected:
  //// Display names
  QMap<QString/*email*/, QString/*dispalyname*/> DisplayNames;

  //// Ids
  QMap<QString/*email*/, int/*id*/> Ids;
};
