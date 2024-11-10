#ifndef LEGEND_GROUP_CREATOR_H
#define LEGEND_GROUP_CREATOR_H

#include <ui/dialogs/templates/dialog.h>
#include <logic/models/analysis_entity_model.h>

namespace Ui {
class LegendGroupCreator;
}
class ColorItemModel;
class ShapeItemModel;
namespace Dialogs
{
class LegendGroupCreator : public Templates::Dialog
{
  Q_OBJECT

public:
  explicit LegendGroupCreator( const QList<TEntityUid>& entityUids, QPointer<AnalysisEntityModel> model, bool manualHotteling = false, QWidget *parent = nullptr);
  ~LegendGroupCreator();

  // WebDialog interface
public slots:
  void Accept() override;
  void Reject() override;

private:
  void setupUi();
  void onGroupComboIndexChanged(int ind);
  void setCurrentItem(const TEntityUid& uid);

private:
  QList<TEntityUid> mEntityUids;
  QPointer<AnalysisEntityModel> mModel;
  QPointer<ColorItemModel> mColorModel;
  QPointer<ShapeItemModel> mShapeModel;
  Ui::LegendGroupCreator *ui;
  QWidget* mContent;
  bool mInManualHotteling;

};
}//namespace Dialogs

#endif // LEGEND_GROUP_CREATOR_H
