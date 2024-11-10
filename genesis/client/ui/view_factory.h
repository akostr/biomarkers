#pragma once

#include <QObject>
#include <QWidget>

/////////////////////////////////////////////////////////////
//// Fwds
namespace Views
{
class View;
}
/////////////////////////////////////////////////////////////
//// View factory
class ViewFactory : public QObject
{
  Q_OBJECT

public:
  ViewFactory(QObject* parent = nullptr);
  virtual ~ViewFactory();

  //// Shall be implemented
  virtual Views::View* CreateView(QWidget* parent) = 0;

  //// Get created cached view
  virtual Views::View* GetView() = 0;
};
