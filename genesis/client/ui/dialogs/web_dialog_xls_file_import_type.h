#ifndef WEB_DIALOG_XLS_FILE_IMPORT_TYPE_H
#define WEB_DIALOG_XLS_FILE_IMPORT_TYPE_H

#include "ui/dialogs/templates/dialog.h"
#include "logic/enums.h"

namespace Ui {
class WebDialogXlsFileImportType;
}

namespace Dialogs
{
class WebDialogXlsFileImportType : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit WebDialogXlsFileImportType(QWidget* parent = nullptr);
  ~WebDialogXlsFileImportType();

public slots:
  Import::XlsFileImportedType importType();

private:
  Ui::WebDialogXlsFileImportType* ui;

  void setupUi();
};
}

#endif // WEB_DIALOG_XLS_FILE_IMPORT_TYPE_H
