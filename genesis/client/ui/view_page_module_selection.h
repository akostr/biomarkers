#ifndef VIEW_PAGE_MODULE_SELECTION_H
#define VIEW_PAGE_MODULE_SELECTION_H

#include <ui/view.h>

namespace Ui {
class ViewPageModulesSelection;
}

class ModulePlate;
namespace Views
{
class ViewPageModulesSelection : public View
{
  Q_OBJECT

public:
  explicit ViewPageModulesSelection(QWidget *parent = nullptr);
  ~ViewPageModulesSelection();

  // View interface
public:
  void ApplyContextUser(const QString &dataId, const QVariant &data) override;

private:
  Ui::ViewPageModulesSelection *ui;
  QPointer<ModulePlate> ReservoirPlate;
  QPointer<ModulePlate> BiomarkersPlate;
  QPointer<ModulePlate> PlotsPlate;
};
}//namespace Views
#endif // VIEW_PAGE_MODULE_SELECTION_H
