#include "app.h"

#include "../api/network_manager.h"
#include "known_context_tag_names.h"
#include "notification.h"
#include <ui/genesis_window.h>

#include <genesis_style/app_proxy_style.h>

#include <QTranslator>
#include <qsettings.h>
#include <QVariantAnimation>
#include <QPainter>
#include <QVector2D>

using namespace Core;

namespace
{
  static GenesisApp* GenesisApp_Instance__ = nullptr;
  static QSettings AppSettings("config.cfg", QSettings::IniFormat);

  static QString APPLICATION_NAME = "AppName";
  static QString APP_URL = "WebAppUrl";
  static QString KEYCLOAK_URL = "KeycloakUrl";
  static QString CLIENT_ID = "client_id";
  static QString TEST_U = "test_u";
  static QString TEST_P = "test_p";

  QVariant pixmapInterpolator(const QPixmap& start, const QPixmap& end, qreal progress);
  QVariant vector2dInterpolator(const QVector2D& start, const QVector2D& end, qreal progress);
}

  ////////////////////////////////////////////////////
//// Genesis Application class
GenesisApp::GenesisApp(int& argc, char** argv)
  : QApplication(argc, argv)
{
  GenesisApp_Instance__ = this;

  //// Set up global app objects
  Setup();
}

GenesisApp::~GenesisApp()
{
  GenesisApp_Instance__ = nullptr;
}

static void InitSettings()
{

#ifdef PLATFORM_WASM
  AppSettings.setValue(APP_URL, WASM_APP_URL);
  AppSettings.setValue(CLIENT_ID, WASM_CLIENT_ID);
  AppSettings.setValue(KEYCLOAK_URL, WASM_KEYCLOAK_URL);
#else

  #ifdef GENESIS_APP_URL
    if(AppSettings.value(APP_URL).toString().isEmpty())
    {
      AppSettings.setValue(APP_URL, GENESIS_APP_URL);
    }
  #endif

  #ifdef GENESIS_CLIENT_ID
    if(AppSettings.value(CLIENT_ID).toString().isEmpty())
    {
      AppSettings.setValue(CLIENT_ID, GENESIS_CLIENT_ID);
    }
  #endif

  #ifdef GENESIS_KEYCLOAK_URL
    if(AppSettings.value(KEYCLOAK_URL).toString().isEmpty())
    {
      AppSettings.setValue(KEYCLOAK_URL, GENESIS_KEYCLOAK_URL);
    }
  #endif

#endif

  if(AppSettings.value(APPLICATION_NAME).toString().isEmpty())
  {
    AppSettings.setValue(APPLICATION_NAME, "Geohimia");
  }
}

static void LoadSettings()
{

}

//// Get app instance
GenesisApp* GenesisApp::Get()
{
  return GenesisApp_Instance__;
}

//// Initial setup
void GenesisApp::Setup()
{
  //// Set writable location before translator as settings are required
//  for (const auto& key : { HOST, PORT, PORT_SQL })
//  {
//    if (!AppSettings.contains(key))
//    {
//      InitSettings();
//      break;
//    }
//  }

  qRegisterAnimationInterpolator<QPixmap>(pixmapInterpolator);
  qRegisterAnimationInterpolator<QVector2D>(vector2dInterpolator);

  InitSettings();

  setApplicationName(AppSettings.value(APPLICATION_NAME).toString());

  //// Translator
  {
    QString lang = "ru";
    QString country = "RU";

    QStringList translatorFiles;
    translatorFiles << QString(":/linguist/qt_%1.qm").arg(lang);
    translatorFiles << QString(":/linguist/qtbase_%1.qm").arg(lang);
    translatorFiles << QString(":/linguist/project_%1_%2.qm").arg(lang).arg(country);
    for (const auto& file : translatorFiles)
    {
      QTranslator* translator = new QTranslator(this);
      if (translator->load(file))
      {
        installTranslator(translator);
      }
      else
      {
        delete translator;
      }
    }
  }

  //// Ui
#ifndef DISABLE_STYLES
  setStyle(new AppPropxyStyle);
#endif

  //// Root context
  RootContext.reset(new GenesisContextRoot(nullptr));


  const QString app_url = AppSettings.value(APP_URL).toString();
  const QString keycloak_url = AppSettings.value(KEYCLOAK_URL).toString();
  const QString client_id = AppSettings.value(CLIENT_ID).toString();


#ifndef PLATFORM_WASM
  #ifndef PUBLIC
  if(!AppSettings.value(TEST_U).toString().isEmpty())
  {
    RootContext.get()->GetContextUser()->SetData(Names::UserContextTags::kLogin, AppSettings.value(TEST_U).toString());
  }
  if(!AppSettings.value(TEST_P).toString().isEmpty())
  {
    RootContext.get()->GetContextUser()->SetData(Names::UserContextTags::kPassword, AppSettings.value(TEST_P).toString());
  }
  #endif
#endif

  //// Newtwork manager
  auto netManager = NetworkManager::Get();
  netManager->SetHost(app_url);
  netManager->SetKeyCloak(keycloak_url, client_id);

  netManager->connect(netManager, &NetworkManager::reloginRequired, RootContext.get(), [this]()
  {
    auto userContext = RootContext->GetContextUser();
    userContext->SetData(Names::UserContextTags::kInitialPageId, Views::View::CurrentPageId());
    userContext->SetData(Names::UserContextTags::kUserState, Names::UserContextTags::notAuthorized);
#ifdef PUBLIC
    userContext->SetData(Names::UserContextTags::kPassword, "");
#endif

    Notification::NotifyWarning(tr("Relogin, to continue work"), tr("Session expired"));
  });
}

//// Get root context
QPointer<GenesisContextRoot> GenesisApp::GetRootContext()
{
  return RootContext.data();
}

namespace
{
QVariant pixmapInterpolator(const QPixmap& start, const QPixmap& end, qreal progress)
{
  auto lerpSize = [](QSizeF a, QSizeF b, qreal progress) -> QSizeF
  {return a + (b - a) * progress;};
  auto lerp = [](qreal a, qreal b, qreal progress) -> qreal
  {return a + (b - a) * progress;};

  QPixmap blend(lerpSize(start.size(), end.size(), progress).toSize());
  blend.fill(QColor(0,0,0,0));
  QPainter p(&blend);
  p.drawPixmap(blend.rect(), start);
  p.setOpacity(progress);
  p.drawPixmap(blend.rect(), end);
  blend.setDevicePixelRatio(lerp(start.devicePixelRatio(), end.devicePixelRatio(), progress));
  return blend;
}
QVariant vector2dInterpolator(const QVector2D& start, const QVector2D& end, qreal progress)
{
  if(progress == 1)
    return end;
  if(progress == 0)
    return start;

  return start + (end - start) * progress;
}

}
