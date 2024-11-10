#include "genesis_window.h"
#include "view_factory_registry.h"
#include "view_factory_impls.h"

#include "../logic/context_root.h"
#include <logic/known_context_tag_names.h>
#include "ui/known_view_names.h"
#include "ui/view_root.h"
#include "dialogs/web_overlay.h"

#include <extern/common_core/settings/settings.h>

#include <QPushButton>
#include <QApplication>
#include <QMouseEvent>

#ifndef PLATFORM_WASM
#include <QProcess>
#endif

//using namespace Core;
using namespace Names;
using namespace ViewPageNames;

////////////////////////////////////////////////////
//// Main window class
namespace Details
{
  GenesisWindow* GenesisWindow_Instance__ = 0;
}

////////////////////////////////////////////////////
//// Main window class
GenesisWindow::GenesisWindow(QWidget* parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
{
  //// Instance
  setObjectName("GenesisRootWindow");
  Details::GenesisWindow_Instance__ = this;

  //// Create view factories
  new ViewFactoryRegistry(this);

  //// Setup
  SetupUi();

  qApp->installEventFilter(this);
}

GenesisWindow::~GenesisWindow()
{
  Common::Settings::Get().SetValue("GenesisWindow/geometry", QString::fromStdString(saveGeometry().toBase64().toStdString()));

  Details::GenesisWindow_Instance__ = nullptr;
}

GenesisWindow* GenesisWindow::Get()
{
  return Details::GenesisWindow_Instance__;
}

//// Show page
void GenesisWindow::ShowPage(const QString& factoryId)
{
  //// Pages shall be switched through context data
  if(auto root = qobject_cast<ViewRoot*>(RootView))
  {
    if(auto currentPage = root->GetCurrentPage())
      currentPage->SwitchToPage(factoryId);
    else
      root->SwitchToPage(factoryId);
  }
}

void GenesisWindow::ShowPage(const std::string &factoryId)
{
  ShowPage(_Q(factoryId));
}

void GenesisWindow::SwitchModule(int module)
{
  //// Modules shall be switched through context data
  if(auto root = qobject_cast<ViewRoot*>(RootView))
  {
    if(auto currentPage = root->GetCurrentPage())
      currentPage->SwitchToModule(module);
    else
      root->SwitchToModule(module);
  }
}

QPointer<View> GenesisWindow::CurrentPage()
{
  if(auto root = qobject_cast<ViewRoot*>(RootView))
  {
    if(auto currentPage = root->GetCurrentPage())
      return currentPage;
    else
      return root;
  }
  else
    //should never been here
    return nullptr;
}

QUuid GenesisWindow::ShowOverlay(const QString& text)
{
  auto id = QUuid::createUuid();
  Overlays.append({ id, new WebOverlay(text, this) });
  return id;
}

void GenesisWindow::RemoveOverlay(QUuid id)
{
  for (int i = 0; i < Overlays.size(); ++i)
  {
    if (Overlays[i].first == id)
    {
      if (auto overlay = Overlays[i].second)
      {
        delete overlay;
        Overlays.removeAt(i);
        return;
      }
    }
  }
}

void GenesisWindow::UpdateOverlay(QUuid id, const QString &text)
{
  for (int i = 0; i < Overlays.size(); ++i)
  {
    if (Overlays[i].first == id)
    {
      if (auto overlay = Overlays[i].second)
      {
        Overlays[i].second->setText(text);
        return;
      }
    }
  }
}

void GenesisWindow::SetLogFilename(const QString filename)
{
#ifndef PLATFORM_WASM
  current_log_filename = filename;
  keyLogs = new QShortcut(this);   // Initialize the object
  keyLogs->setKey(Qt::CTRL | Qt::Key_L);    // Set the key code
  connect(keyLogs, &QShortcut::activated, this, [this]
  {
      QProcess::startDetached("C:\\Windows\\system32\\notepad.exe", {current_log_filename});
  },
  Qt::QueuedConnection);
#endif
}

void GenesisWindow::SetupUi()
{
  //// Register all view factories
  ViewFactoryImpls::InitializeViewFactories();

  //// Self
  setMinimumSize(950, 650);

  //// Root
  Root = new QWidget(this);
  setCentralWidget(Root);

  RootLayout = new QStackedLayout(Root);
  RootLayout->setStackingMode(QStackedLayout::StackAll);
  RootLayout->setContentsMargins(0, 0, 0, 0);

  //// Root view
  {
    RootView = View::Create(ViewRootPageName, Root);
    RootLayout->addWidget(RootView);
    RootLayout->setCurrentWidget(RootView);

    //// Set
    setContentsMargins(0, 0, 0, 0);
    setCentralWidget(Root);

    //// Restore geometry
    QByteArray geometry = QByteArray::fromBase64(QByteArray::fromStdString(Common::Settings::Get().GetValue("GenesisWindow/geometry").toString().toStdString()));
    if (!geometry.isEmpty())
    {
      restoreGeometry(geometry);
    }
  }

  //// Notifications layer
  {
    Notifications = new NotificationPresenter(Root);
    RootLayout->addWidget(Notifications);
    RootLayout->setCurrentWidget(Notifications);
  }
}


bool GenesisWindow::eventFilter(QObject *watched, QEvent *event)
{
  if((event->type() == QEvent::MouseMove) &&
      isVisible() &&
      QString(watched->metaObject()->className()) == "QWidgetWindow")
  {
    auto e = (QMouseEvent*)event;
    QPointF windowPos = e->scenePosition();
    emit windowMouseMove(windowPos);
  }
  return false;
}
