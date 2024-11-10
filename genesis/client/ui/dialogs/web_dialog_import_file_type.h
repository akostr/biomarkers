#ifndef WEB_DIALOG_IMPORT_FILE_TYPE_H
#define WEB_DIALOG_IMPORT_FILE_TYPE_H

#include <ui/dialogs/templates/dialog.h>
#include <logic/enums.h>

namespace Ui {
class WebDialogImportFileType;
}

using Btns = QDialogButtonBox::StandardButton;
namespace Dialogs
{
class WebDialogImportFileType : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit WebDialogImportFileType(QWidget *parent);
  ~WebDialogImportFileType();

public slots:
  Import::ImportedFilesType fileType();

private:
  void setupUi();

private:
  Ui::WebDialogImportFileType *ui;
};
}//namespace Dialogs
#endif // WEB_DIALOG_IMPORT_FILE_TYPE_H
