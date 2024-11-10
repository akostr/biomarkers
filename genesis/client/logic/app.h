#pragma once

#include <QApplication>
#include <QSharedPointer>
#include <QPointer>

#include "context_root.h"

////////////////////////////////////////////////////
//// Genesis Application class
class GenesisApp : public QApplication
{
  Q_OBJECT

public:
  GenesisApp(int& argc, char** argv);
  ~GenesisApp();

  //// Get app instance
  static GenesisApp* Get();

  //// Get root context
  QPointer<Core::GenesisContextRoot> GetRootContext();

private:
  //// Initial setup
  void Setup();

private:
  //// Root app context
  QSharedPointer<Core::GenesisContextRoot> RootContext;
};
