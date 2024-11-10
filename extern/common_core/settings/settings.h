#pragma once

#include <QObject>
#include <QVariant>
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>

namespace Common
{
  //// Fwds
  class DynamicSettingsAccessor;

  //////////////////////////////////////////////////
  //// Centralized Settings 
  class Settings : public QObject
  {
    Q_OBJECT

  public:
    Settings();
    ~Settings();
    
  public:
    static Settings& Get();
    static void      Shutdown();

#ifndef DISABLE_SETTINGS_WRITABLE_LOCATION
    void Load();
    void Save();
    void SaveSectionToFile(QString sectionPath, QString filePath);
    void LoadSectionFromFile(QString sectionPath, QString filePath);
#endif // ~!DISABLE_SETTINGS_WRITABLE_LOCATION

    //// Static values
    QVariant GetValue(const QString& path, const QVariant& defaultValue = QVariant());
    void SetValue(const QString& path, const QVariant& value);

    QVariantMap GetSection(const QString& path);
    void SetSection(const QString& path, const QVariantMap& value);

    void RemoveSection(const QString& path);

    //// Dynamic values
    QVariant GetDynamicValue(const QUuid& accessorId, const QString& path, const QVariant& defaultValue = QVariant()) const;
    void SetDynamicValue(const QUuid& accessorId, const QString& path, const QVariant& value);

    QVariantMap GetDynamicSection(const QUuid& accessorId, const QString& path) const;
    void SetDynamicSection(const QUuid& accessorId, const QString& path, const QVariantMap& value);

    void RemoveDynamicSection(const QUuid& accessorId, const QString& path);

    //// Dynamic accessors
    void RegisterDynamicAccessor(const QUuid& accessorId, DynamicSettingsAccessor* accessor);
    DynamicSettingsAccessor* GetDynamicAccessor(const QUuid& accessorId) const;
    void RemoveDynamicAccessor(const QUuid& accessorId);

  private:
    QVariantMap GetSection(QVariantMap *root, const QString& path) const;
    void SetSection(QVariantMap *root, const QString& path, const QVariantMap& value);
    void RemoveSection(QVariantMap *root, const QString& path);

  private:
    QString         SettingsLocation;
    QString         FileName;
    QVariantMap     Root;

    QMap<QUuid, DynamicSettingsAccessor*> DynamicSettingsAccessors;
  };

  //// Dynamic Settings Accessor
  class DynamicSettingsAccessor
  {
  public:
    DynamicSettingsAccessor();
    virtual ~DynamicSettingsAccessor();

    virtual QVariantMap& GetSettingsObject() = 0;
    virtual void        SetSettingsObject(const QVariantMap& object) = 0;

    void SetId(const QUuid& id);
    void Register();
    void Unregister();

  protected:
    QUuid Id;
  };
};
