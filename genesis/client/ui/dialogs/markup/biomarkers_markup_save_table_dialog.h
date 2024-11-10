#ifndef BIOMARKERS_MARKUP_SAVE_TABLE_DIALOG_H
#define BIOMARKERS_MARKUP_SAVE_TABLE_DIALOG_H

#include <ui/dialogs/templates/info.h>
#include <QUuid>

namespace Ui {
class BiomarkersMarkupSaveTableDialog;
}

class TreeModelDynamicNumericTables;
class QStandardItemModel;
class BiomarkersMarkupSaveTableDialog : public Dialogs::Templates::Info
{
  Q_OBJECT

public:
  explicit BiomarkersMarkupSaveTableDialog(QWidget *parent, const QStringList& occupiedNames);
  ~BiomarkersMarkupSaveTableDialog();
  bool isExistedTableGroup();
  bool isCommonGroup();
  int tableGroupId();
  QString tableGroupTitle();
  QString tableTitle();
  QString tableComment();

  // WebDialog interface
public slots:
  void Accept() override;

private:
  void setupUi();
  void onTablesModelReset();
  void onCurrentPageIndexChanged(int current);
  void onRadioGroupIdToggled(int id, bool state);

private:
  Ui::BiomarkersMarkupSaveTableDialog *ui;
  QWidget* mContent;
  QPointer<TreeModelDynamicNumericTables> mTablesModel;
  QPointer<QStandardItemModel> mComboModel;
  QPointer<QWidget> mLoadingOverlay;
  QPointer<QPushButton> mStepBackBtn;
  QStringList mOccupiedNames;

};

#endif // BIOMARKERS_MARKUP_SAVE_TABLE_DIALOG_H
