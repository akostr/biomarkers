#ifndef WEBDIALOGINPUTDIALOG_H
#define WEBDIALOGINPUTDIALOG_H

#include "web_dialog.h"

#include <QLineEdit>

class WebDialogInputDialog : public WebDialog
{
  Q_OBJECT
public:
  enum DataType
  {
    Invalid,
    String
  };
  WebDialogInputDialog(QString caption, QString subCaption, DataType type, QWidget* parent);
  void setupUi();
  DataType getDataType();
  QVariant getValue();

private:
  void setupStringInput();

  DataType m_type;
  QPointer<QLineEdit> m_stringEdit;
  QString m_caption;
  QString m_subCaption;
};

#endif // WEBDIALOGINPUTDIALOG_H
