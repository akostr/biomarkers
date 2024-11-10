#ifndef VIEWPAGEPROFILE_H
#define VIEWPAGEPROFILE_H

#include "view.h"

namespace Ui {
class ProfilePage;
}

namespace Views
{
class ViewPageProfile : public View
{
  Q_OBJECT
public:
  ViewPageProfile(QWidget* parent = nullptr);
  ~ViewPageProfile();
  void SetupUi();

protected:
  Ui::ProfilePage* ui;

private:
  void reloadData();

  // View interface
public:
  void ApplyContextUser(const QString& dataId, const QVariant& data) override;
};
}//namespace Views
#endif // VIEWPAGEPROFILE_H
