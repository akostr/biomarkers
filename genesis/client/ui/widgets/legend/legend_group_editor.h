#ifndef LEGEND_GROUP_EDITOR_H
#define LEGEND_GROUP_EDITOR_H

#include <ui/dialogs/templates/dialog.h>
#include <logic/models/analysis_entity_model.h>

namespace Ui {
class LegendGroupEditor;
}
namespace Dialogs
{
class LegendGroupEditor : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit LegendGroupEditor( const TEntityUid& entityUid, QPointer<AnalysisEntityModel> model, QWidget *parent = nullptr);
  ~LegendGroupEditor();

signals:
  void removeGroupAndRegroup(const TEntityUid& groupUid);

  // WebDialog interface
public slots:
  void Accept() override;
  void Reject() override;

private:
  void setupUi();
  void removeGroup();
  void showConfirmDialog();

private:
  QUuid mEntityUid;
  QPointer<AnalysisEntityModel> mModel;
  Ui::LegendGroupEditor *ui;
  QWidget* mContent;

};
}//namespace Dialogs
#endif // LEGEND_GROUP_EDITOR_H
