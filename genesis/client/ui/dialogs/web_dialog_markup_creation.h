#ifndef WEBDIALOGMARKUPCREATION_H
#define WEBDIALOGMARKUPCREATION_H


#include <ui/dialogs/templates/save_edit.h>

class QLineEdit;
class QLabel;
class FlowLayout;
class WebOverlay;
namespace Dialogs
{
class WebDialogMarkupCreation : public Templates::SaveEdit
{
  Q_OBJECT
public:
  WebDialogMarkupCreation(QList<int> filesIds, QWidget* parent);
  ~WebDialogMarkupCreation();

  void SetupUi();
  QString GetMarkupName();
  QString GetComment();
  QList<int> GetFilesIds();

private:
  QList<int> FilesIds;

private:
  void loadMarkupsList(WebOverlay* overlay = nullptr);
};
}//namespace Dialogs
#endif // WEBDIALOGMARKUPCREATION_H
