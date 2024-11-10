#ifndef VIEWPAGEPROJECTMODELSLIST_H
#define VIEWPAGEPROJECTMODELSLIST_H

#include "view.h"

class QVBoxLayout;
class QFrame;
class QTableView;
class TreeModelDynamicProjectModelsList;
namespace Views
{
class ViewPageProjectModelsList : public View
{
  Q_OBJECT
public:
  ViewPageProjectModelsList(QWidget* parent = 0);
  ~ViewPageProjectModelsList();

  void SetupUi();

private:
  //// Logic
  QPointer<TreeModelDynamicProjectModelsList> Models;

  //// Ui
  QPointer<QVBoxLayout>        Layout;
  QPointer<QTableView>         Table;

private:
  void UpdateTitleAndCommentOnServer(int id, const QString& title,
                                     const QString& comment);
};

#endif // VIEWPAGEPROJECTMODELSLIST_H
}//namespace Views
