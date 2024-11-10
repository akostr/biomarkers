#include "settings.h"

#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
#include <utils/json_utils/files.h>
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

#include <QDir>
#include <QStandardPaths>
#include <QSharedPointer>

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
#include <licensing/licensing.h>
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define VARIANT_TYPE_ID typeId
#else
#define VARIANT_TYPE_ID type
#endif

namespace Common
{
  namespace
  {
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    QString SettingsLocationSuffix__
      =
#ifdef SETTINGS_WRITABLE_LOCATION_KEY
      "/" SETTINGS_WRITABLE_LOCATION_KEY "/";
#else
      "/CyberFrac/";
#endif
    QString SettingsFileName__ = "settings.json";
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

    QSharedPointer<Settings> Settings_Instance__ = nullptr;
    class SetingsStaticInitializer
    {
    public:
      SetingsStaticInitializer()
      {
        Settings::Get();
      }
    } SetingsStaticInitializer_Instance;
  }

  //////////////////////////////////////////////////
  //// Centralized Settings 
  Settings::Settings()
  {
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    {
      QString SettingsLocation = QStandardPaths::writableLocation(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      QStandardPaths::AppLocalDataLocation
      #else
      QStandardPaths::DataLocation
#endif
      );
      SettingsLocation += SettingsLocationSuffix__;
      QDir::root().mkpath(SettingsLocation);
      FileName = SettingsLocation + SettingsFileName__;
      
      Load();
    }
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
    CET_LICENSE_CHECK_OR_DELETE(this);
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
  }

  Settings::~Settings()
  {
#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    Save();
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION
  }
  
  Settings& Settings::Get()
  {
    if (!Settings_Instance__)
      Settings_Instance__.reset(new Settings);
    return *Settings_Instance__;
  }

  void Settings::Shutdown()
  {
    Settings_Instance__.reset();
  }

#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
  void Settings::Load()
  {
    QJsonDocument doc;
    JsonUtils::Files::ReadJsonFile(FileName, doc);
    Root = doc.object().toVariantMap();
  }

  void Settings::Save()
  {
    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(Root));
    JsonUtils::Files::WriteJsonFile(FileName, doc);
  }

  void Settings::SaveSectionToFile(QString sectionPath, QString filePath)
  {
    QJsonDocument doc;
    doc.setObject(QJsonObject::fromVariantMap(GetSection(sectionPath)));
    JsonUtils::Files::WriteJsonFile(filePath, doc);
  }

  void Settings::LoadSectionFromFile(QString sectionPath, QString filePath)
  {
    QJsonDocument doc;
    JsonUtils::Files::ReadJsonFile(filePath, doc);
    SetSection(sectionPath, doc.object().toVariantMap());
  }
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

  QVariant Settings::GetValue(const QString& path, const QVariant& defaultValue)
  {
    QVariantMap object = GetSection(path);
    if (object.contains("value"))
      return object["value"];
    return defaultValue;
  }

  void Settings::SetValue(const QString& path, const QVariant& value)
  {
    QVariantMap object;
    object["value"] = value;
    SetSection(path, object);
  }

  QVariantMap Settings::GetSection(const QString& path)
  {
    return GetSection(&Root, path);
  }

  void Settings::SetSection(const QString& path, const QVariantMap& value)
  {
    return SetSection(&Root, path, value);
  }

  void Settings::RemoveSection(const QString& path)
  {
    RemoveSection(&Root, path);
  }

  QVariantMap Settings::GetSection(QVariantMap *root, const QString& path) const
  {
    if (path.isEmpty())
      return *root;
  
    QStringList nodeNames = path.split('/');
    QString outerNodeName = nodeNames.takeFirst();

    if (root->contains(outerNodeName))
    {
      QVariant *innerObject = &(root->operator [](outerNodeName));
      if (innerObject->VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      {
        QVariantMap* innerMap = reinterpret_cast<QVariantMap*>(innerObject);
        QString innerPath = nodeNames.join('/');
        return GetSection(innerMap, innerPath);
      }
    }
    return QVariantMap();
  }

  void Settings::SetSection(QVariantMap *root, const QString& path, const QVariantMap& value)
  {
    if (path.isEmpty())
    {
      *root = value;
    }
    else
    {
      QStringList nodeNames = path.split('/');
      QString outerNodeName = nodeNames.takeFirst();

      if (!root->contains(outerNodeName))
        root->operator [](outerNodeName) = QVariantMap();
      QVariant* innerObject = &(root->operator [](outerNodeName));
      if (innerObject->VARIANT_TYPE_ID() != QMetaType::QVariantMap)
      {
        root->operator [](outerNodeName) = QVariantMap();
        innerObject = &(root->operator [](outerNodeName));
      }
      if (innerObject->VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      {
        QVariantMap* innerMap = reinterpret_cast<QVariantMap*>(innerObject);
        QString innerPath = nodeNames.join('/');
        SetSection(innerMap, innerPath, value);
      }
    }
  }

  void Settings::RemoveSection(QVariantMap *root, const QString& path)
  {
    if (!path.contains('/'))
    {
      root->remove(path);
    }
    else
    {
      QStringList nodeNames = path.split('/');
      QString outerNodeName = nodeNames.takeFirst();

      if (root->contains(outerNodeName))
      {
        QVariant *innerObject = &(root->operator [](outerNodeName));
        if (innerObject->VARIANT_TYPE_ID() == QMetaType::QVariantMap)
        {
          QVariantMap* innerMap = reinterpret_cast<QVariantMap*>(innerObject);
          QString innerPath = nodeNames.join('/');
          RemoveSection(innerMap, innerPath);
        }
      }
    }
  }
  
  //// Dynamic values
  QVariant Settings::GetDynamicValue(const QUuid &accessorId, const QString &path, const QVariant &defaultValue) const
  {
    auto accessor = GetDynamicAccessor(accessorId);
    if (accessor)
    {
      QVariantMap* configuration = &accessor->GetSettingsObject();
      return GetSection(configuration, path).value("value", defaultValue);
    }
    return defaultValue;
  }

  void Settings::SetDynamicValue(const QUuid &accessorId, const QString &path, const QVariant &value)
  {
    auto accessor = GetDynamicAccessor(accessorId);
    if (accessor)
    {
      QVariantMap* configuration = &accessor->GetSettingsObject();
      QVariantMap object;
      object["value"] = value;
      SetSection(configuration, path, object);
    }
  }

  QVariantMap Settings::GetDynamicSection(const QUuid &accessorId, const QString& path) const
  {
    auto accessor = GetDynamicAccessor(accessorId);
    if (accessor)
    {
      QVariantMap* configuration = &accessor->GetSettingsObject();
      return GetSection(configuration, path);
    }
    return QVariantMap();
  }

  void Settings::SetDynamicSection(const QUuid &accessorId, const QString& path, const QVariantMap& value)
  {
    auto accessor = GetDynamicAccessor(accessorId);
    if (accessor)
    {
      QVariantMap* configuration = &accessor->GetSettingsObject();
      SetSection(configuration, path, value);
    }
  }

  void Settings::RemoveDynamicSection(const QUuid &accessorId, const QString &path)
  {
    auto accessor = GetDynamicAccessor(accessorId);
    if (accessor)
    {
      QVariantMap* configuration = &accessor->GetSettingsObject();
      RemoveSection(configuration, path);
    }
  }

  //// Dynamic accessors
  void Settings::RegisterDynamicAccessor(const QUuid& accessorId, DynamicSettingsAccessor* accessor)
  {
    DynamicSettingsAccessors[accessorId] = accessor;
  }

  DynamicSettingsAccessor* Settings::GetDynamicAccessor(const QUuid& accessorId) const
  {
    return DynamicSettingsAccessors.value(accessorId);
  }
  
  void Settings::RemoveDynamicAccessor(const QUuid& accessorId)
  {
    DynamicSettingsAccessors.remove(accessorId);
  }

  //////////////////////////////////////////////////
  //// Dynamic Settings Accessor
  DynamicSettingsAccessor::DynamicSettingsAccessor()
  {
  }

  DynamicSettingsAccessor::~DynamicSettingsAccessor()
  {
    Unregister();
  }

  void DynamicSettingsAccessor::SetId(const QUuid& id)
  {
    Unregister();
    Id = id;
    Register();
  }

  void DynamicSettingsAccessor::Register()
  {
    Settings::Get().RegisterDynamicAccessor(Id, this);
  }

  void DynamicSettingsAccessor::Unregister()
  {
    Settings::Get().RemoveDynamicAccessor(Id);
  }
};
