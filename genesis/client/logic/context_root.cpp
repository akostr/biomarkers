#include "context_root.h"

#include "known_context_names.h"
#include "known_context_tag_names.h"

#include <QJsonDocument>

#include <extern/common_core/settings/settings.h>

#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
#include <genesis_crypt/InnerCrypt.h>
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

using namespace Names;

namespace Details
{
  static Core::GenesisContextRootPtr GenesisContextRoot_Instance__ = nullptr;
}

///////////////////////////////////////////////////////////////////
//// Genesis context root
namespace Core
{
  GenesisContextRoot::GenesisContextRoot(GenesisContext* parent)
    : GenesisContext(parent)
  {
    Details::GenesisContextRoot_Instance__ = this;

    //// Class tag
    SetClass(GenesisContextNames::Root);

    //// Child contexts
    Setup();

    //// Load
    LoadSettings();
  }

  GenesisContextRoot::~GenesisContextRoot()
  {
    //// Save
    SaveSettings();

    Details::GenesisContextRoot_Instance__ = nullptr;
  }

  //// Instance
  GenesisContextRootPtr GenesisContextRoot::Get()
  {
    return Details::GenesisContextRoot_Instance__;
  }

  int GenesisContextRoot::ProjectId()
  {
    return Get()->GetContextProject()->GetData(Names::ContextTagNames::ProjectId).toInt();
  }

  //// Setup initial structure
  void GenesisContextRoot::Setup()
  {
    GetContextUi();
    GetContextUser();
    GetContextModules();
    GetContextProject();
    GetContextFileCDF();
    GetContextMarkup();
    GetContextMarkupVersion();
    GetContextMarkupVersionAnalisysPCA();
    GetContextStepBasedMarkup();
    GetContextMarkupVersionAnalisysMCR();
    GetContextMarkupVersionAnalisysPLS();
    GetContextMarkupVersionAnalisysPLSPredict();
    GetContextPCAPlots();
  }

  //// Get child context that describes current ui state
  GenesisContextPtr GenesisContextRoot::GetContextUi()
  {
    //// Child of root
    return GetChildContextUi();
  }

  //// Get child context that describes current user
  GenesisContextPtr GenesisContextRoot::GetContextUser()
  {
    //// Child of root
    return GetChildContext(GenesisContextNames::User);
  }

  GenesisContextPtr GenesisContextRoot::GetContextModules()
  {
    return GetChildContextModules();
  }

  //// Get child context that describes current project
  GenesisContextPtr GenesisContextRoot::GetContextProject()
  {
    //// Child of root
    return GetChildContext(GenesisContextNames::Project);
  }

  //// Get child context that describes current file
  GenesisContextPtr GenesisContextRoot::GetContextFileCDF()
  {
    //// Child of project
    return GetContextProject()->GetChildContext(GenesisContextNames::FileCDF);
  }

  //// Get child context that describes current project markup
  GenesisContextPtr GenesisContextRoot::GetContextMarkup()
  {
    //// Child of project
    return GetContextProject()->GetChildContext(GenesisContextNames::Markup);
  }

  //// Get child context that describes current project markup version
  GenesisContextPtr GenesisContextRoot::GetContextMarkupVersion()
  {
    //// Child of merkup
    return GetContextMarkup()->GetChildContext(GenesisContextNames::MarkupVersion);
  }

  GenesisContextStepBasedMarkupPtr GenesisContextRoot::GetContextStepBasedMarkup()
  {
    return GetChildContextStepBasedMarkup();
  }

  GenesisContextPtr GenesisContextRoot::GetContextHeightRatioMatrix()
  {
    return GetContextMarkup()->GetChildContext(GenesisContextNames::RatioMatrix);
  }

  //// Get child context that describes current project markup version data
  GenesisContextPtr GenesisContextRoot::GetContextMarkupVersionAnalisysPCA()
  {
    //// Child of merkup version
    return GetContextMarkupVersion()->GetChildContext(GenesisContextNames::MarkupVersionAnalysisPCA);
  }

  GenesisContextPtr GenesisContextRoot::GetContextMarkupVersionAnalisysMCR()
  {
    //// Child of merkup version
    return GetContextMarkupVersion()->GetChildContext(GenesisContextNames::MarkupVersionAnalysisMCR);
  }

  GenesisContextPtr GenesisContextRoot::GetContextMarkupVersionAnalisysPLS()
  {
    //// Child of merkup version
    return GetContextMarkupVersion()->GetChildContext(GenesisContextNames::MarkupVersionAnalysisPLS);
  }

  GenesisContextPtr GenesisContextRoot::GetContextMarkupVersionAnalisysPLSPredict()
  {
    return GetContextMarkupVersion()->GetChildContext(GenesisContextNames::MarkupVersionAnalysisPLSPredict);
  }

  GenesisContextPtr GenesisContextRoot::GetContextPCAPlots()
  {
    return GetContextProject()->GetChildContext(GenesisContextNames::PCAPlots);
  }

  GenesisContextPtr GenesisContextRoot::GetContextLibrary()
  {
    return GetContextUser()->GetChildContext(GenesisContextNames::Library);
  }

  GenesisContextPtr GenesisContextRoot::GetCreationPlot()
  {
    return GetContextUser()->GetChildContext(GenesisContextNames::CreationPlot);
  }

  //// Save settings
  void GenesisContextRoot::SaveSettings()
  {
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    //// Storage
    QJsonObject data;

    //// Credentials
//    data["user.email"] = GetContextUser()->GetData(Names::UserContextTags::kEmail).toString();
    data["user.login"] = GetContextUser()->GetData(Names::UserContextTags::kLogin).toString();
    data["user.password"] = GetContextUser()->GetData(Names::UserContextTags::kPassword).toString();

    //// Store encrypted
    QJsonDocument document;
    document.setObject(data);

    QByteArray json = document.toJson();

    InnerCrypt crypto;
    QByteArray jsonEncrypted = crypto.Encrypt(json);

    QByteArray jsonEncrypted64 = jsonEncrypted.toBase64();
    QString    jsonValue = QString::fromStdString(jsonEncrypted64.toStdString());

    //// Store
    Common::Settings::Get().SetValue("GenesisContextRoot/root/crypto", jsonValue);
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
  }

  //// Load setiings
  void GenesisContextRoot::LoadSettings()
  {
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    //// Load
    QString jsonValue = Common::Settings::Get().GetValue("GenesisContextRoot/root/crypto").toString();

    //// Read
    QByteArray jsonEncrypted64 = QByteArray::fromStdString(jsonValue.toStdString());
    QByteArray jsonEncrypted = QByteArray::fromBase64(jsonEncrypted64);

    InnerCrypt crypto;
    QByteArray json = crypto.Decrypt(jsonEncrypted);

    QJsonDocument document = QJsonDocument::fromJson(json);

    QJsonObject data = document.object();

    GetContextUser()->SetData(Names::UserContextTags::kLogin, data["user.login"].toString());
//    GetContextUser()->SetData(Names::UserContextTags::kEmail, data["user.email"].toString());
    GetContextUser()->SetData(Names::UserContextTags::kPassword, data["user.password"].toString());
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
  }
}
