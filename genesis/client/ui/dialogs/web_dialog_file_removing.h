#ifndef WEB_DIALOG_FILE_REMOVING_H
#define WEB_DIALOG_FILE_REMOVING_H

#include "web_dialog.h"
#include <QMap>

namespace Ui {
class WebDialogFileRemoving;
}
using Btns = QDialogButtonBox::StandardButton;

class WebDialogFileRemovingConfirmation : public WebDialog
{
  Q_OBJECT

public:
  explicit WebDialogFileRemovingConfirmation(const QList<QPair<int, QString> > &projectFilesInfos, QWidget *parent);
  ~WebDialogFileRemovingConfirmation();

private:
  void setupUi();
  void queryComplete();

private:
  Ui::WebDialogFileRemoving *ui;
  QList<QPair<int, QString>> m_projectFilesInfos;
  QMap<int, QString> m_fileIdToInfoMap;
  int m_remainingQueries;
};

#endif // WEB_DIALOG_FILE_REMOVING_H
