#pragma once

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
#include <rapidjson/include/rapidjson/document.h>
#include <rapidjson/include/rapidjson/rapidjson.h>
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

#include <QVariant>
#include <QVariantMap>
#include <QSize>
#include <QRecursiveMutex>
#include <QPointer>
#include <QSet>

#include "standard_item_model_base.h"
#include "uom.h"

////////////////////////////////////////////////////////////
//// Standard Item Model
class StandardItemModel : public StandardItemModelBase
{
  Q_OBJECT

public:
  //// Serializable Index Path
  class IndexPath
  {
  public:
    IndexPath(const QModelIndex& target);
    IndexPath(const QStringList& segments);
    IndexPath(const QString& path);

    QModelIndex GetIndex(QAbstractItemModel* model);
    QModelIndex GetIndex(QAbstractItemModel* model, const QModelIndex& root, const QStringList& segments);

    QString     GetPath();

  private:
    QStringList Segments;
  };

public: 

  typedef StandardItem          Item;
  typedef StandardItemModelBase Inherited;
  typedef QMap<int, QString>    ColumnKeys;

public:
#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  //// Rapid / variant helpers
  static QVariant         RapidJsonToQVariant(const rapidjson::Value& value);
  static rapidjson::Value QVariantToRapidJson(const QVariant& variant, rapidjson::Document::AllocatorType& allocator);
#endif

public:
  //// Predefined user data roles
  enum UserRoles
  {
    ////////////////////////////////////////////////////////////////////////////////////
    //// StandardItemModel specific roles         CODES BELOW MUST REMAIN UNCHANGED
    UserRoleIconAsString                        = Qt::UserRole + 1,   //// -> affects     Qt::DecorationRole    | icon path string
    UserRoleKeyForValue                         = Qt::UserRole + 2,   //// -> stored in   UserRoleKeyForValue   | key substituted for value recieved from editor for predefined values
    UserRoleBackgroundAsString                  = Qt::UserRole + 3,   //// -> affects     Qt::BackgroundRole    | background as string containing color code
    UserRoleForegroundAsString                  = Qt::UserRole + 4,   //// -> affects     Qt::ForegroundRole    | foreground as string containing color code
    UserRoleFontAsString                        = Qt::UserRole + 5,   //// -> affects     Qt::FontRole          | font as string "Familiy,pointSizeInt,weightInt,italicBool" as in QFont ctor

    ////////////////////////////////////////////////////////////////////////////////////
    //// Dynamic column key role
    UserRoleColumnKey                           = Qt::UserRole + 6,   //// Dynamic column key may be returned for specific index

    //// Clipboard interaction roles
    UserRoleClipboardDataRoleId                 = Qt::UserRole + 7,   //// Qt data role id used for clipboard interaction, e.g. UserRoleClipboardDataRole (otherwise Qt::DisplayRole, Qt::EditRole are used)
    UserRoleClipboardDataRole                   = Qt::UserRole + 8,   //// Clipboard interaction role used in both data() and setData()

    ////////////////////////////////////////////////////////////////////////////////////
    //// UOM Roles
    UserRoleUOMSemantics                        = Qt::UserRole + 9,   //// UOM - high level semantics
    UserRoleUOMPhysicalQuantity                 = Qt::UserRole + 10,  //// UOM - physical units
    UserRoleUOMUnitsStored                      = Qt::UserRole + 11,  //// UOM - units in which data is stored
    UserRoleUOMUnitsUI                          = Qt::UserRole + 12,  //// UOM - units in which data is displayed

    UserRoleSourceModelInformationRole          = Qt::UserRole + 13,  //// May be overridden by proxy so that proxybecomes source of model information tags

    UserRoleStringSizeHint                      = Qt::UserRole + 14,
    
    UserRoleUOMConvertFromUI                    = Qt::UserRole + 15,  //// When used in setData(), converts ui input value to stored value applicable in Qt::EditRole 
    UserRoleUOMConvertToUI                      = Qt::UserRole + 16,  //// When used in data(), converts stored value from Qt::EditRole to value applicable in UI
                                                                      //// Equivalent to Qt::DiplayRole 

    UserRoleLast                                = 1000,               //// Keep persistent for models that derive user roles from UserRoleLast for stored values
  };

public:
  //// Model Descriptor
  struct ModelDescriptor
  {
    //// Save / load from variant map
    virtual bool IsNull() const = 0;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) = 0;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const = 0;
  };

  //// Descriptors / Model / Validation
  struct ModelValidation : public ModelDescriptor
  {
  public:
    ModelValidation();
    ModelValidation(QAbstractItemModel* model, const QVariantMap& validation);
    ModelValidation(QAbstractItemModel* model);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

    virtual void Save(QAbstractItemModel* model) const;

    //// Validation by uniquness
    struct RecordUniqueness : public ModelDescriptor
    {
    public:
      RecordUniqueness();
      RecordUniqueness(QAbstractItemModel* model, const QVariantMap& properties);
      RecordUniqueness(QAbstractItemModel* model);

      virtual bool IsNull() const override;
      virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
      virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

    public:
      int     Column;
      QString ValueName;
      QString ValueDefaultName;
    };

  public:
    RecordUniqueness Uniqueness;
  };

  //// Descriptors / Model / Record duplication rules
  struct ModelDuplication : public ModelDescriptor
  {
  public:
    ModelDuplication();
    ModelDuplication(QAbstractItemModel* model, const QVariantMap& duplication);
    ModelDuplication(QAbstractItemModel* model);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

    virtual void Save(QAbstractItemModel* model) const;

  public:
    bool        CanDuplicate;
    QList<int>  Duplicatable;
  };

  //// Descriptors / Model / Column / Key
  struct ColumnKey : public ModelDescriptor
  {
    ColumnKey();
    ColumnKey(int integer, const QString& string);
    ColumnKey(QAbstractItemModel* model, int integer);
    ColumnKey(QAbstractItemModel* model, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    int     Integer;
    QString String;
  };

  //// Descriptors / Model / Column or Row / format
  struct GenericFormat : public ModelDescriptor
  {
  public:
    GenericFormat();
    GenericFormat(QAbstractItemModel* model, const QVariantMap& properties);
    GenericFormat(QAbstractItemModel* model, bool column, int integer);
    GenericFormat(QAbstractItemModel* model, bool column, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    int     Decimals;
    int     DecimalsDisplay;
    double  Step;
    bool    Scientific;
    bool    IconDisplay;
  };

  //// Descriptors / Model / Column / Format
  struct ColumnFormat : public GenericFormat
  {
  public:
    ColumnFormat();
    ColumnFormat(QAbstractItemModel* model, const QVariantMap& properties);
    ColumnFormat(QAbstractItemModel* model, int integer);
    ColumnFormat(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Row / Format
  struct RowFormat : public GenericFormat
  {
  public:
    RowFormat();
    RowFormat(QAbstractItemModel* model, const QVariantMap& properties);
    RowFormat(QAbstractItemModel* model, int integer);
    RowFormat(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Column or Row / Validation
  struct GenericValidation : public ModelDescriptor
  {
  public:
    GenericValidation();
    GenericValidation(QAbstractItemModel* model, const QVariantMap& properties);
    GenericValidation(QAbstractItemModel* model, bool column, int integer);
    GenericValidation(QAbstractItemModel* model, bool column, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    double  Minimum;
    double  Maximum;
    double  ExclusiveMinimum;
    double  ExclusiveMaximum;
    double  LocalMinimum;
    double  LocalMaximum;
    double  InputMinimum;
    double  InputMaximum;
  };

  //// Descriptors / Model / Column / Validation
  struct ColumnValidation : public GenericValidation
  {
  public:
    ColumnValidation();
    ColumnValidation(QAbstractItemModel* model, const QVariantMap& properties);
    ColumnValidation(QAbstractItemModel* model, int integer);
    ColumnValidation(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Row / Validation
  struct RowValidation : public GenericValidation
  {
  public:
    RowValidation();
    RowValidation(QAbstractItemModel* model, const QVariantMap& properties);
    RowValidation(QAbstractItemModel* model, int integer);
    RowValidation(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Column / Scheme
  struct ColumnScheme : public ModelDescriptor
  {
  public:
    ColumnScheme();
    ColumnScheme(QAbstractItemModel* model, const QVariantMap& properties);
    ColumnScheme(QAbstractItemModel* model, int integer);
    ColumnScheme(QAbstractItemModel* model, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    QMetaType::Type Type;
    QVariant        TypicalValue;
  };

  //// Descriptors / Model / Column / External data connection
  struct ColumnDataConnection : public ModelDescriptor
  {
  public:
    ColumnDataConnection();
    ColumnDataConnection(QAbstractItemModel* model, const QVariantMap& properties);
    ColumnDataConnection(QAbstractItemModel* model, int integer);
    ColumnDataConnection(QAbstractItemModel* model, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    QPointer<QAbstractItemModel> SourceModelPtr;  // Source (external) model pointer
    QPointer<QAbstractItemModel> GroupModelPtr; // groups for it

    //// Required to copy recored from more than one column
    QMap<int, QString> SourceColumnKeys;

    //// Required to copy recored from more than one column, overrides ColumnKeys if necessary
    QMap<int, QString> TargetColumnKeys;

    //// Aliases
    QMap<QString, QString> Aliases;   // e.g. ["Connection::Key"] = "Key"

    //// Source data interaction flags
    bool Select;
    bool Edit;

    //// Calculated fields
    int SourceIndexKey;               // Index of a column in source (external) model containing QUuid to be stored in target
    int SourceIndexName;              // Index of a column in source (external) model containing QString to represent in editor

    //// Calculated fields
    QMap<QString, int> SourceColumnIndexes;
    QMap<QString, int> TargetColumnIndexes;
  };

  //// Descriptors / Model / Column or Row / Predefined values
  struct GenericPredefinedValues : public ModelDescriptor
  {
  public:
    GenericPredefinedValues();
    GenericPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties);
    GenericPredefinedValues(QAbstractItemModel* model, bool column, int integer);
    GenericPredefinedValues(QAbstractItemModel* model, bool column, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    QVariantMap Values;               // e.g. ["My value description"] = "MyValueKey"
    QVariantMap Order;                // e.g. ["My value description"] = 0
    bool        Editable;
  };

  //// Descriptors / Model / Column / Predefined values
  struct ColumnPredefinedValues : public GenericPredefinedValues
  {
  public:
    ColumnPredefinedValues();
    ColumnPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties);
    ColumnPredefinedValues(QAbstractItemModel* model, int integer);
    ColumnPredefinedValues(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Row / Predefined values
  struct RowPredefinedValues : public GenericPredefinedValues
  {
  public:
    RowPredefinedValues();
    RowPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties);
    RowPredefinedValues(QAbstractItemModel* model, int integer);
    RowPredefinedValues(QAbstractItemModel* model, const QString& string);
  };

  //// Descriptors / Model / Column / UOM
  struct ColumnUOM : public ModelDescriptor
  {
  public:
    ColumnUOM();
    ColumnUOM(QAbstractItemModel* model, int integer);
    ColumnUOM(QAbstractItemModel* model, const QString& string);

    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    ColumnKey               Key;
    QString                 Semantics;
    UOM::PhysicalQuantity   Quantity;
    int                     UnitsStored;
    int                     UnitsUI;
  };

  //// Descriptors / Model / Column / Visual
  struct ColumnVisual : public ModelDescriptor
  {
  public:
    ColumnVisual();
    ColumnVisual(QAbstractItemModel* model, int integer);
    ColumnVisual(QAbstractItemModel* model, const QString& string);
    ColumnVisual(QAbstractItemModel* model, const QVariantMap& properties);

  public:
    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    QSize       SizeHint;
  };

  //// Full Column Info
  struct ColumnInformation : public ModelDescriptor
  {
  public:
    ColumnInformation();
    ColumnInformation(QAbstractItemModel* model, int integer);
    ColumnInformation(QAbstractItemModel* model, const QString& string);
    ColumnInformation(QAbstractItemModel* model, const QVariantMap& properties, const ColumnKey& key);

  public:
    virtual bool IsNull() const override;  
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

    virtual void Save(QAbstractItemModel* model) const;

  public:
    ColumnKey               Key;
    ColumnFormat            Format;
    ColumnValidation        Validation;
    ColumnScheme            Scheme;
    ColumnDataConnection    DataConnection;
    ColumnPredefinedValues  Values;
    ColumnUOM               UOM;
    ColumnVisual            Visual;
    bool                    Editable;
  };

  //// Row information (for transposed models)
  struct RowInformation : public ModelDescriptor
  {
  public:
    RowInformation();
    RowInformation(QAbstractItemModel* model, const QVariantMap& properties, int integer);
    RowInformation(QAbstractItemModel* model, int integer);

  public:
    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;

  public:
    int                   Key;      // Rows shall be addressed by integer index only
    RowFormat             Format;
    RowValidation         Validation;
    RowPredefinedValues   Values;
  };

  //// Legacy keys resolver (must preceed ModelInformation ctor)
  struct ModelInformationLegacyKeysResolver : public ModelDescriptor
  {
  public:
    ModelInformationLegacyKeysResolver(QAbstractItemModel* model);
  };

  //// Full Model Info
  struct ModelInformation : public ModelInformationLegacyKeysResolver
  {
  public:
    ModelInformation();
    ModelInformation(QAbstractItemModel* model);
    ModelInformation(QAbstractItemModel* model, const QVariantMap& properties);

  public:

    //// Helper
    class ColumnInformationMap : public QMap<QString, ColumnInformation>
    {
    public:
      ColumnInformationMap(const QMap<int, QString>& keys)
        : Keys(keys)
      {
      }

      using QMap<QString, ColumnInformation>::operator[];

      ColumnInformation& operator[](int i)
      {
        if (Keys.contains(i))
          return QMap<QString, ColumnInformation>::operator[](Keys.value(i));
        return QMap<QString, ColumnInformation>::operator[](QString::number(i));
      }
      const ColumnInformation operator[](int i) const
      {
        if (Keys.contains(i))
          return QMap<QString, ColumnInformation>::operator[](Keys.value(i));
        return QMap<QString, ColumnInformation>::operator[](QString::number(i));
      }
    private:
      const QMap<int, QString>& Keys;
    };

  public:
    virtual bool IsNull() const override;
    virtual void Load(QAbstractItemModel* model, const QVariantMap& properties) override;
    virtual void Save(QAbstractItemModel* model, QVariantMap& properties) const override;
    
    virtual void Load(QAbstractItemModel* model);
    virtual void Save(QAbstractItemModel* model) const;

  public:
    //// Column keys
    QMap<int, QString>                  ColumnKeys;                 // e.g. [0]     = "Key"

    //// Global string keys aliases
    QMap<QString, QString>              Aliases;                    // e.g. ["Connection::Key"] = "Key"

    //// Column Specific
    ColumnInformationMap                Columns;

    //// Row Specific (format, values, - shall override corresponding column, rows settings shall be indexed by indeger key only)
    QMap<int, RowInformation>           Rows;

    //// General for all model
    ModelValidation                     Validation;
    ModelDuplication                    Duplication;

    //// Calculated fields, dont fill them manually
    QMap<QString, int>                  ColumnIndexes;              // e.g. ["Key"] = 0
    QMap<QString, QString>              ColumnKeysStringAliases;    // e.g. ["0"]   = "Key", ["Key"] = "Key", ["Connection::Key"] = "Key"
  };

public:

  //////////////////////////////////////////////////////////////////////////
  //// Keys / Model (Structured)
  static const char* KeyModelInformation;
  static const char* KeyModelInformationValidation;
  static const char* KeyModelInformationDuplication;
  static const char* KeyModelKeyColumnId;

  static const char* KeyValidation;
  static const char* KeyValidationUnique;
  static const char* KeyValidationUniqueValue;
  static const char* KeyValidationUniqueValueName;
  static const char* KeyValidationUniqueValueDefaultName;

  static const char* KeyDuplication;
  static const char* KeyDuplicationCanDuplicate;
  static const char* KeyDuplicationDuplicatable;

  static const char* KeyModelInformationColumnKeys;
  static const char* KeyModelInformationAliases;
  
  static const char* KeyModelInformationColumnInformation;
  static const char* KeyModelInformationRowInformation;

  //////////////////////////////////////////////////////////////////////////
  //// Keys / Column (Structured)
  static const char* KeyColumnInformation;

  static const char* KeyColumnInformationKey;
  static const char* KeyColumnInformationKeyInteger;
  static const char* KeyColumnInformationKeyString;

  static const char* KeyColumnInformationFormat;
  static const char* KeyFormat;
  static const char* KeyFormatDecimals;
  static const char* KeyFormatDecimalsDisplay;
  static const char* KeyFormatStep;
  static const char* KeyFormatScience;
  static const char* KeyFormatIconDisplay;

  static const char* KeyColumnInformationValidation;
  static const char* KeyColumnValidation;
  static const char* KeyColumnValidationMinimun;
  static const char* KeyColumnValidationMaximum;
  static const char* KeyColumnValidationExclusiveMinimum;
  static const char* KeyColumnValidationExclusiveMaximum;
  static const char* KeyColumnValidationLocalMinimum;
  static const char* KeyColumnValidationLocalMaximum;
  static const char* KeyColumnValidationInputMinimum;
  static const char* KeyColumnValidationInputMaximum;

  static const char* KeyColumnInformationScheme;

  static const char* KeyScheme;
  static const char* KeySchemeType;
  static const char* KeySchemeTypicalValue;

  static const char* KeyColumnInformationDataConnection;
  static const char* KeyDataConnection;
  static const char* KeyDataConnectionSourceModelPtr;
  static const char* KeyDataConnectionGroupModelPtr;
  static const char* KeyDataConnectionSourceColumnKeys;
  static const char* KeyDataConnectionSourceIndexKey;
  static const char* KeyDataConnectionSourceIndexName;
  static const char* KeyDataConnectionTargetColumnKeys;
  static const char* KeyDataConnectionAliases;
  static const char* KeyDataConnectionSelect;
  static const char* KeyDataConnectionEdit;

  static const char* KeyColumnInformationEditable;

  static const char* KeyColumnInformationPredefinedValues;
  static const char* KeyPredefinedValues;
  static const char* KeyPredefinedValuesValues;
  static const char* KeyPredefinedValuesOrder;
  static const char* KeyPredefinedValuesEditable;

  static const char* KeyColumnInformationVisual;
  static const char* KeyVisual;
  static const char* KeyVisualSizeHintWidth;
  static const char* KeyVisualSizeHintHeight;

  //// Keys / Rows (structured)
  static const char* KeyRowInformation;
  static const char* KeyRowInformationKey;
  static const char* KeyRowInformationFormat;
  static const char* KeyRowInformationValidation;
  static const char* KeyRowInformationPredefinedValues;

public:
  explicit StandardItemModel(QObject* parent = nullptr);
  StandardItemModel(int rows, int columns, QObject* parent = nullptr);
  ~StandardItemModel();

public:
  //// Normalize keys (structured from legacy)
  static void               NormalizeKeys(QAbstractItemModel* model, bool force = false, Qt::ConnectionType connection = Qt::AutoConnection);
  static void               NormalizeKeysImpl(QAbstractItemModel* model, bool force = false);

  //// Static accessors for models that dont inherit StandardItemModel
  static ModelInformation   GetModelInformation(QAbstractItemModel* model);
  static void               SetModelInformation(QAbstractItemModel* model, const ModelInformation& modelInformation);

  static ColumnInformation  GetColumnInformation(QAbstractItemModel* model, const int& index);
  static ColumnInformation  GetColumnInformation(QAbstractItemModel* model, const QString& key);

  static void               SetColumnInformation(QAbstractItemModel* model, const ColumnInformation& columnInformation);

  static void               SetColumnInformation(QAbstractItemModel* model, const int& index, const ColumnInformation& columnInformation);
  static void               SetColumnInformation(QAbstractItemModel* model, const QString& key, const ColumnInformation& columnInformation);

  //// Static / Column keys
  static QVariantMap        GetColumnKeys(QAbstractItemModel* model);
  static void               SetColumnKeys(QAbstractItemModel* model, const QVariantMap& keys);
  static void               SetColumnKeys(QAbstractItemModel* model, const QMap<int, QString>& keys);

  //// Static / Specific column key
  static void               SetColumnKey(QAbstractItemModel* model, const int& column, const QString& key);
  static int                GetColumnIndex(QAbstractItemModel* model, const QString& key);

  static QString            GetStaticColumnKey(QAbstractItemModel* model, const int& key);
  static QString            GetDynamicColumnKey(QAbstractItemModel* model, const QModelIndex& mmodelIndex);

  static QString            GetColumnKey(const QVariantMap& columnKeys, const int& key);
  static int                GetColumnIndex(const QVariantMap& columnKeys, const QString& key);

  //// Notifications
  static void               NotifyModelInformationChanged(QAbstractItemModel* model);

public:
  //////////////////////////////////////////////////////////////////////
  //// Model info
  ModelInformation  GetModelInformation() const;
  void              SetModelInformation(const ModelInformation& modelInformation);

  //////////////////////////////////////////////////////////////////////////
  //// Column info
  ColumnInformation GetColumnInformation(const int& index) const;
  ColumnInformation GetColumnInformation(const QString& key) const;

  //// Generics
  GenericValidation       GetColumnValidation(const QModelIndex& modelIndex) const;
  GenericPredefinedValues GetColumnPredefinedValues(const QModelIndex& modelIndex) const;

  //////////////////////////////////////////////////////////////////////////
  //// Column keys
  QVariantMap       GetColumnKeys() const;
  QStringList       GetStaticColumnKeys() const;

  void              SetColumnKeys(const QVariantMap& keys);
  void              SetColumnKeys(const QMap<int, QString>& keys);

  //// Specific column key
  void              SetColumnKey(const int& column, const QString& key);
  int               GetColumnIndex(const QString& key) const;
  
  virtual QString   GetColumnKey(const QModelIndex& index) const; 

  virtual QString   GetStaticColumnKey(const QModelIndex& modelIndex) const;
  virtual QString   GetStaticColumnKey(const int& key) const;
  virtual QString   GetDynamicColumnKey(const QModelIndex& modelIndex) const;

  //// Column editable flag
  void              SetColumnEditable(int column, bool editable);
  void              SetColumnEditable(const QString& key, bool editable);

  //// Column properties / Format
  void              SetColumnFormat(const QString& key, const QVariantMap& format);
  void              SetColumnFormat(int key, const QVariantMap& format);
  void              SetColumnFormat(const QString& key, const ColumnFormat& format);
  void              SetColumnFormat(int key, const ColumnFormat& format);

  //// Model validation rules
  ModelValidation   GetValidation() const;
  void              SetValidation(const ModelValidation& validation);

  //// Model record duplication rules
  ModelDuplication  GetDuplication() const;
  void              SetDuplication(const ModelDuplication& duplication);

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  //// Invoke Save / Load with specified connection type
  void              Load(const rapidjson::Value& json, Qt::ConnectionType connection);
  void              Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator, Qt::ConnectionType connection) const;
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

public:
  //////////////////////////////////////////////////////////////////////
  //// Reimplemented
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual bool     setData(const QModelIndex& index, const QVariant& data, int role = Qt::EditRole) override;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  virtual bool     CanAcceptClipboardData(const QModelIndex& index, const ColumnInformation& columnInformation, const QVariant& data);

  //////////////////////////////////////////////////////////////////////
  //// Index to string / from string
  QString           GetKeyFromModelIndex(const QModelIndex& index) const;
  QModelIndex       GetModelIndexFromKey(const QString& key, const QModelIndex& parent = QModelIndex()) const;

public slots:
  //// Normalize keys
  Q_INVOKABLE virtual void NormalizeKeys(bool force = false);
  
#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  //// Save / Load, override theese to save / load auxiliary data
  Q_INVOKABLE virtual void Load(const rapidjson::Value& json);
  Q_INVOKABLE virtual void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const;

private:
  std::function<void(const rapidjson::Value&)>                                auxiliaryLoader;
  std::function<void(rapidjson::Value&, rapidjson::Document::AllocatorType&)> auxiliarySaver;
public:
  void SetAuxiliaryLoader(std::function<void(const rapidjson::Value&)>);
  void SetAuxiliarySaver (std::function<void(rapidjson::Value&, rapidjson::Document::AllocatorType&)>);
  std::function<void(const rapidjson::Value&)>                                AuxiliaryLoader() const;
  std::function<void(rapidjson::Value&, rapidjson::Document::AllocatorType&)> AuxiliarySaver () const;
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

  //// Loading flag
  Q_INVOKABLE virtual void SetLoading(bool loading);
  Q_INVOKABLE virtual bool GetLoading() const;

  //// Lock / Unlock (save / load high level methods)
  void Lock();
  void Unlock();

  //// Reset
  Q_INVOKABLE virtual void Reset(); 
  Q_INVOKABLE virtual void SortRows(const QModelIndex& parent, int column);

  //// Notifications
  void NotifyModelInformationChanged();

public:
#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
  void Load(const QModelIndex& index, const rapidjson::Value& json);
  void LoadRow(const QModelIndex& index, int r, const rapidjson::Value& json);

  void Save(const QModelIndex& index, rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const;
  void SaveRow(const QModelIndex& index, int r, rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const;
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

  virtual void FillItemDataMap(const QModelIndex& mi, QVariantMap& dataMap) const;
  
  void SetKeyColumnId(const int v);
  int GetKeyColumnId() const;

  static void Clone(QAbstractItemModel* const source, const QModelIndex& sourceIndex, QAbstractItemModel* const target, const QModelIndex& targetIndex);
  void Clone(const QModelIndex& sourceIndex, const QModelIndex& targetIndex);
signals:
  void ModelInformationChanged();
  void KeyDataChanged(QModelIndex, QVariant before, QVariant after);

protected:
  mutable QRecursiveMutex SaveLoadMutex;
  QSet<int>               StoredRoles;

private:
  
};
