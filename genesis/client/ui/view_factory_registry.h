#pragma once

#include "view_factory.h"

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QPointer>

/////////////////////////////////////////////////////////////
//// Fwds
namespace Views
{
  class View;
}
class ViewFactory;

/////////////////////////////////////////////////////////////
//// View factory registry
class ViewFactoryRegistry : public QObject
{
  Q_OBJECT

public:
  explicit ViewFactoryRegistry(QObject* parent);
  ~ViewFactoryRegistry();

  //// Get instance
  static ViewFactoryRegistry* Get();

  //// Access factories
  void         RegisterFactory(const QString& type, ViewFactory* factory);
  ViewFactory* GetFactory(const QString& type);

private:
  QMap<QString, QPointer<ViewFactory>> Factories;
};
