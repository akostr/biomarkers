#include "standard_item_model.h"
#include "standard_uom_model.h"
#include "standard_uom_model.h"
#include "abstract_uom_model.h"
#include "uom_settings.h"

#include <QAbstractProxyModel>
#include <QCoreApplication>
#include <QThread>
#include <QReadLocker>
#include <QWriteLocker>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>
#include <QFile>
#include <QTimer>

#include <set>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define VARIANT_TYPE_ID typeId
#else
#define VARIANT_TYPE_ID type
#endif


////////////////////////////////////////////////////////////
//// Standard Item Model Keys
namespace Legacy
{
  const char* LegacyKeyColumnKeys                          = "ColumnKeys";
  const char* LegacyKeyAliases                             = "Aliases";

  const char* LegacyKeyEditable                            = "Editable";

  const char* LegacyKeyFormat                              = "Format";
  const char* LegacyKeyFormatDecimals                      = "Decimals";
  const char* LegacyKeyFormatDecimalsDisplay               = "DecimalsDisplay";
  const char* LegacyKeyFormatStep                          = "Step";
  const char* LegacyKeyFormatMinimun                       = "Minimun";
  const char* LegacyKeyFormatMaximum                       = "Maximum";
  const char* LegacyKeyFormatExclusiveMinimum              = "ExclusiveMinimum";
  const char* LegacyKeyFormatExclusiveMaximum              = "ExclusiveMaximum";
  const char* LegacyKeyFormatLocalMinimum                  = "LocalMinimum";
  const char* LegacyKeyFormatLocalMaximum                  = "LocalMaximum";
  const char* LegacyKeyFormatScience                       = "Science";
  const char* LegacyKeyFormatIsForRows                     = "FormatIsForRows";
  const char* LegacyKeyFormatIconDisplay                   = "IconDisplay";

  const char* LegacyKeyValidation                          = "Validation";
  const char* LegacyKeyValidationUniqueValue               = "UniqueValue";
  const char* LegacyKeyValidationUniqueValueName           = "UniqueValueName";
  const char* LegacyKeyValidationUniqueValueDefaultName    = "UniqueValueDefaultName";
  const char* LegacyKeyValidationIsForRows                 = "ValidationIsForRows";

  const char* LegacyKeyDuplication                         = "Duplication";
  const char* LegacyKeyDuplicationCanDuplicate             = "CanDuplicate";
  const char* LegacyKeyDuplicationDuplicatable             = "Duplicatable";

  const char* LegacyKeyDataConnections                     = "DataConnections";
  const char* LegacyKeyDataConnectionsSourceModelPtr       = "SourceModelPtr";
  const char* LegacyKeyDataConnectionsGroupModelPtr        = "GroupModelPtr";
  const char* LegacyKeyDataConnectionsSourceColumnKeys     = "SourceColumnKeys";
  const char* LegacyKeyDataConnectionsTargetColumnKeys     = "TargetColumnKeys";
  const char* LegacyKeyDataConnectionsSelect               = "Select";
  const char* LegacyKeyDataConnectionsEdit                 = "Edit";

  const char* LegacyKeyValues                              = "Values";
  const char* LegacyKeyValuesOrder                         = "ValuesOrder";
  const char* LegacyKeyValuesEditable                      = "ValuesEditable";
  const char* LegacyKeyValuesIsForRows                     = "ValuesIsForRows";
}

//////////////////////////////////////////////////////////////////////////  
//// Keys / Model (Structured)
const char* StandardItemModel::KeyModelInformation                          = "StandardItemModel::ModelInformation";
const char* StandardItemModel::KeyModelInformationValidation                = "StandardItemModel::ModelInformation::Validation";
const char* StandardItemModel::KeyModelInformationDuplication               = "StandardItemModel::ModelInformation::Duplication";
const char* StandardItemModel::KeyModelKeyColumnId                          = "StandardItemModel::ModelInformation::KeyColumn";

const char* StandardItemModel::KeyValidation                                = StandardItemModel::KeyModelInformationValidation;
const char* StandardItemModel::KeyValidationUnique                          = "StandardItemModel::ModelInformation::Validation::Unique";
const char* StandardItemModel::KeyValidationUniqueValue                     = "StandardItemModel::ModelInformation::Validation::Unique::UniqueValue";
const char* StandardItemModel::KeyValidationUniqueValueName                 = "StandardItemModel::ModelInformation::Validation::Unique::UniqueValueName";
const char* StandardItemModel::KeyValidationUniqueValueDefaultName          = "StandardItemModel::ModelInformation::Validation::Unique::UniqueValueDefaultName";

const char* StandardItemModel::KeyDuplication                               = StandardItemModel::KeyModelInformationDuplication;
const char* StandardItemModel::KeyDuplicationCanDuplicate                   = "StandardItemModel::ModelInformation::Duplication::CanDuplicate";
const char* StandardItemModel::KeyDuplicationDuplicatable                   = "StandardItemModel::ModelInformation::Duplication::Duplicatable";

const char* StandardItemModel::KeyModelInformationColumnKeys                = "StandardItemModel::ModelInformation::ColumnKeys";
const char* StandardItemModel::KeyModelInformationAliases                   = "StandardItemModel::ModelInformation::Aliases";

const char* StandardItemModel::KeyModelInformationColumnInformation         = "StandardItemModel::ColumnInformation";
const char* StandardItemModel::KeyModelInformationRowInformation            = "StandardItemModel::RowInformation";

//////////////////////////////////////////////////////////////////////////  
//// Keys / Column (Structured)
const char* StandardItemModel::KeyColumnInformation                         = KeyModelInformationColumnInformation;

const char* StandardItemModel::KeyColumnInformationKey                      = "StandardItemModel::ColumnInformation::Key";
const char* StandardItemModel::KeyColumnInformationKeyInteger               = "StandardItemModel::ColumnInformation::Key::Integer";
const char* StandardItemModel::KeyColumnInformationKeyString                = "StandardItemModel::ColumnInformation::Key::String";

const char* StandardItemModel::KeyColumnInformationFormat                   = "StandardItemModel::ColumnInformation::Format";

const char* StandardItemModel::KeyFormat                                    = StandardItemModel::KeyColumnInformationFormat;
const char* StandardItemModel::KeyFormatDecimals                            = "StandardItemModel::ColumnInformation::Format::Decimals";
const char* StandardItemModel::KeyFormatDecimalsDisplay                     = "StandardItemModel::ColumnInformation::Format::DecimalsDisplay";
const char* StandardItemModel::KeyFormatStep                                = "StandardItemModel::ColumnInformation::Format::Step";
const char* StandardItemModel::KeyFormatScience                             = "StandardItemModel::ColumnInformation::Format::Science";
const char* StandardItemModel::KeyFormatIconDisplay                         = "StandardItemModel::ColumnInformation::Format::IconDisplay";

const char* StandardItemModel::KeyColumnInformationValidation               = "StandardItemModel::ColumnInformation::Validation";
const char* StandardItemModel::KeyColumnValidation                          = StandardItemModel::KeyColumnInformationValidation;
const char* StandardItemModel::KeyColumnValidationMinimun                   = "StandardItemModel::ColumnInformation::Validation::Minimun";
const char* StandardItemModel::KeyColumnValidationMaximum                   = "StandardItemModel::ColumnInformation::Validation::Maximum";
const char* StandardItemModel::KeyColumnValidationExclusiveMinimum          = "StandardItemModel::ColumnInformation::Validation::ExclusiveMinimum";
const char* StandardItemModel::KeyColumnValidationExclusiveMaximum          = "StandardItemModel::ColumnInformation::Validation::ExclusiveMaximum";
const char* StandardItemModel::KeyColumnValidationLocalMinimum              = "StandardItemModel::ColumnInformation::Validation::LocalMinimum";
const char* StandardItemModel::KeyColumnValidationLocalMaximum              = "StandardItemModel::ColumnInformation::Validation::LocalMaximum";
const char* StandardItemModel::KeyColumnValidationInputMinimum              = "StandardItemModel::ColumnInformation::Validation::InputMinimum";
const char* StandardItemModel::KeyColumnValidationInputMaximum              = "StandardItemModel::ColumnInformation::Validation::InputMaximum";


const char* StandardItemModel::KeyColumnInformationScheme                   = "StandardItemModel::ColumnInformation::Scheme";

const char* StandardItemModel::KeyScheme                                    = StandardItemModel::KeyColumnInformationScheme;
const char* StandardItemModel::KeySchemeType                                = "StandardItemModel::ColumnInformation::Scheme::Type";
const char* StandardItemModel::KeySchemeTypicalValue                        = "StandardItemModel::ColumnInformation::Scheme::TypicalValue";

const char* StandardItemModel::KeyColumnInformationDataConnection           = "StandardItemModel::ColumnInformation::DataConnection";

const char* StandardItemModel::KeyDataConnection                            = StandardItemModel::KeyColumnInformationDataConnection;
const char* StandardItemModel::KeyDataConnectionSourceModelPtr              = "StandardItemModel::ColumnInformation::DataConnection::SourceModelPtr";
const char* StandardItemModel::KeyDataConnectionGroupModelPtr               = "StandardItemModel::ColumnInformation::DataConnection::GroupModelPtr";
const char* StandardItemModel::KeyDataConnectionSourceColumnKeys            = "StandardItemModel::ColumnInformation::DataConnection::SourceColumnKeys";
const char* StandardItemModel::KeyDataConnectionSourceIndexKey              = "StandardItemModel::ColumnInformation::DataConnection::SourceIndexKey";
const char* StandardItemModel::KeyDataConnectionSourceIndexName             = "StandardItemModel::ColumnInformation::DataConnection::SourceIndexName";
const char* StandardItemModel::KeyDataConnectionTargetColumnKeys            = "StandardItemModel::ColumnInformation::DataConnection::TargetColumnKeys";
const char* StandardItemModel::KeyDataConnectionAliases                     = "StandardItemModel::ColumnInformation::DataConnection::Aliases";
const char* StandardItemModel::KeyDataConnectionSelect                      = "StandardItemModel::ColumnInformation::DataConnection::Select";
const char* StandardItemModel::KeyDataConnectionEdit                        = "StandardItemModel::ColumnInformation::DataConnection::Edit";

const char* StandardItemModel::KeyColumnInformationEditable                 = "StandardItemModel::ColumnInformation::Editable";

const char* StandardItemModel::KeyColumnInformationPredefinedValues         = "StandardItemModel::ColumnInformation::PredefinedValues";

const char* StandardItemModel::KeyPredefinedValues                          = StandardItemModel::KeyColumnInformationPredefinedValues;
const char* StandardItemModel::KeyPredefinedValuesValues                    = "StandardItemModel::ColumnInformation::PredefinedValues::Values";
const char* StandardItemModel::KeyPredefinedValuesOrder                     = "StandardItemModel::ColumnInformation::PredefinedValues::Order";
const char* StandardItemModel::KeyPredefinedValuesEditable                  = "StandardItemModel::ColumnInformation::PredefinedValues::Editable";

const char* StandardItemModel::KeyColumnInformationVisual                   = "StandardItemModel::ColumnInformation::Visual";
const char* StandardItemModel::KeyVisual                                    = StandardItemModel::KeyColumnInformationVisual;
const char* StandardItemModel::KeyVisualSizeHintWidth                       = "StandardItemModel::ColumnInformation::Visual::SizeHint::Width";
const char* StandardItemModel::KeyVisualSizeHintHeight                      = "StandardItemModel::ColumnInformation::Visual::SizeHint::Height";

const char* StandardItemModel::KeyRowInformation                            = StandardItemModel::KeyModelInformationRowInformation;
const char* StandardItemModel::KeyRowInformationKey                         = "StandardItemModel::RowInformation::Key";
const char* StandardItemModel::KeyRowInformationFormat                      = "StandardItemModel::RowInformation::Format";
const char* StandardItemModel::KeyRowInformationValidation                  = "StandardItemModel::RowInformation::Validation";
const char* StandardItemModel::KeyRowInformationPredefinedValues            = "StandardItemModel::RowInformation::PredefinedValues";

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
////////////////////////////////////////////////////////////
//// Rapid / variant helpers
QVariant StandardItemModel::RapidJsonToQVariant(const rapidjson::Value& value)
{
  if (value.IsString())
    return QString(value.GetString());
  if (value.IsBool())
    return value.GetBool();
  if (value.IsInt())
    return value.GetInt();
  if (value.IsDouble())
    return value.GetDouble();
  return QVariant();
}

rapidjson::Value StandardItemModel::QVariantToRapidJson(const QVariant& variant, rapidjson::Document::AllocatorType& allocator)
{
  switch (variant.type())
  {
  case QVariant::Bool:
    return rapidjson::Value(variant.toBool());
  case QVariant::Int:
    return rapidjson::Value(variant.toInt());
  case QVariant::UInt:
    return rapidjson::Value(variant.toUInt());
  case QVariant::LongLong:
    return rapidjson::Value(variant.toLongLong());
  case QVariant::ULongLong:
    return rapidjson::Value(variant.toULongLong());
  case QVariant::Double:
    return rapidjson::Value(variant.toDouble());
  case QVariant::String:
    return rapidjson::Value(variant.toString().toStdString(), allocator);
  case QVariant::Uuid:
    return rapidjson::Value(variant.toString().toStdString(), allocator);
  default:
    return rapidjson::Value(variant.toString().toStdString(), allocator);
  }
  return rapidjson::Value();
}
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

////////////////////////////////////////////////////////////
//// Descriptors / Model / Validation
StandardItemModel::ModelValidation::ModelValidation()
  : Uniqueness()
{
}

StandardItemModel::ModelValidation::ModelValidation(QAbstractItemModel* model, const QVariantMap& validation)
  : Uniqueness()
{
  Load(model, validation);
}

StandardItemModel::ModelValidation::ModelValidation(QAbstractItemModel* model)
  : ModelValidation(model, model->property(KeyModelInformation).toMap()[KeyModelInformationValidation].toMap())
{
}

bool StandardItemModel::ModelValidation::IsNull() const
{ 
  return Uniqueness.IsNull();
}

void StandardItemModel::ModelValidation::Load(QAbstractItemModel* model, const QVariantMap& properties)
{
  QVariantMap uniquness = properties[KeyValidationUnique].toMap();
  Uniqueness.Load(model, uniquness);
}

void StandardItemModel::ModelValidation::Save(QAbstractItemModel* model, QVariantMap& properties) const
{
  QVariantMap uniquness;
  Uniqueness.Save(model, uniquness);
  properties[KeyValidationUnique] = uniquness;
}

void StandardItemModel::ModelValidation::Save(QAbstractItemModel* model) const
{
  QVariantMap modelInformation = model->property(KeyModelInformation).toMap();
  
  QVariantMap validation;
  Save(model, validation);

  modelInformation[KeyModelInformationValidation] = validation;
  model->setProperty(KeyModelInformation, modelInformation);
}

//// Descriptors / Model / Validation / RecordUniqueness
StandardItemModel::ModelValidation::RecordUniqueness::RecordUniqueness()
  : Column(-1)
{
}

StandardItemModel::ModelValidation::RecordUniqueness::RecordUniqueness(QAbstractItemModel* model, const QVariantMap& properties)
  : RecordUniqueness()
{
  Load(model, properties);
}

StandardItemModel::ModelValidation::RecordUniqueness::RecordUniqueness(QAbstractItemModel* model)
  : RecordUniqueness(model, model->property(KeyModelInformation).toMap()[KeyModelInformationValidation].toMap()[KeyValidationUnique].toMap())
{
}

bool StandardItemModel::ModelValidation::RecordUniqueness::IsNull() const 
{
  return Column == -1;
}

void StandardItemModel::ModelValidation::RecordUniqueness::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (properties.contains(KeyValidationUniqueValue))
    Column = properties[KeyValidationUniqueValue].toInt();
  if (properties.contains(KeyValidationUniqueValueName))
    ValueName = properties[KeyValidationUniqueValueName].toString();
  if (properties.contains(KeyValidationUniqueValueDefaultName))
    ValueDefaultName = properties[KeyValidationUniqueValueDefaultName].toString();
}

void StandardItemModel::ModelValidation::RecordUniqueness::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyValidationUniqueValue]            = Column;
  properties[KeyValidationUniqueValueName]        = ValueName;
  properties[KeyValidationUniqueValueDefaultName] = ValueDefaultName;
}

//// Descriptors / Model / Duplication
StandardItemModel::ModelDuplication::ModelDuplication()
  : CanDuplicate(true)
  , Duplicatable()
{
}

StandardItemModel::ModelDuplication::ModelDuplication(QAbstractItemModel* model, const QVariantMap& duplication)
  : ModelDuplication()
{
  Load(model, duplication);
}

StandardItemModel::ModelDuplication::ModelDuplication(QAbstractItemModel* model)
  : ModelDuplication(model, model->property(KeyModelInformation).toMap()[KeyModelInformationDuplication].toMap())
{
}

bool StandardItemModel::ModelDuplication::IsNull() const
{
  return Duplicatable.empty();
}

void StandardItemModel::ModelDuplication::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  CanDuplicate = properties[KeyDuplicationCanDuplicate].toBool();
  QVariantList duplicatable = properties[KeyDuplicationDuplicatable].toList();
  for (const auto& d : duplicatable)
  {
    Duplicatable.append(d.toInt());
  }
}

void StandardItemModel::ModelDuplication::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyDuplicationCanDuplicate] = CanDuplicate;
  QVariantList duplicatable;
  for (const auto& d : Duplicatable)
  {
    duplicatable.append(d);
  }
  properties[KeyDuplicationDuplicatable] = duplicatable;
}

void StandardItemModel::ModelDuplication::Save(QAbstractItemModel* model) const
{
  QVariantMap modelInformation = model->property(KeyModelInformation).toMap();

  QVariantMap duplication;
  Save(model, duplication);

  modelInformation[KeyModelInformationDuplication] = duplication;
  model->setProperty(KeyModelInformation, modelInformation);
}

////////////////////////////////////////////////////////////
//// Descriptors / Model / Column / Key
StandardItemModel::ColumnKey::ColumnKey()
  : Integer(-1)
  , String()
{
}

StandardItemModel::ColumnKey::ColumnKey(int integer, const QString& string)
  : Integer(integer)
  , String(string)
{
}

StandardItemModel::ColumnKey::ColumnKey(QAbstractItemModel* model, int integer)
  : ColumnKey()
{
  Integer = integer;
  Load(model, model->property(KeyModelInformation).toMap()[KeyModelInformationColumnKeys].toMap());
}

StandardItemModel::ColumnKey::ColumnKey(QAbstractItemModel* model, const QString& string)
  : ColumnKey()
{
  String = string;
  Load(model, model->property(KeyModelInformation).toMap()[KeyModelInformationColumnKeys].toMap());
}

bool StandardItemModel::ColumnKey::IsNull() const
{
  // keys w/ Interger == -1 are treated as dynamic keys
  return String.isEmpty();
}

void StandardItemModel::ColumnKey::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (!String.isEmpty())
    Integer = StandardItemModel::GetColumnIndex(properties, String);
  else if (Integer != -1)
    String = StandardItemModel::GetColumnKey(properties, Integer);
}

void StandardItemModel::ColumnKey::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  if (!String.isEmpty()) // may save w/ Interger == -1 for dymanic keys
  {
    properties[QString::number(Integer)] = String;
  }
}

//// Descriptors / Model / Column or Row / Validation
StandardItemModel::GenericValidation::GenericValidation()
  : Minimum(std::numeric_limits<double>::quiet_NaN())
  , Maximum(std::numeric_limits<double>::quiet_NaN())
  , ExclusiveMinimum(std::numeric_limits<double>::quiet_NaN())
  , ExclusiveMaximum(std::numeric_limits<double>::quiet_NaN())
  , LocalMinimum(std::numeric_limits<double>::quiet_NaN())
  , LocalMaximum(std::numeric_limits<double>::quiet_NaN())
  , InputMinimum(0)
  , InputMaximum(std::numeric_limits<double>::quiet_NaN())
{
}

StandardItemModel::GenericValidation::GenericValidation(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericValidation()
{
  Load(model, properties);
}

StandardItemModel::GenericValidation::GenericValidation(QAbstractItemModel* model, bool column, int integer)
  : GenericValidation(model, column, column ? StandardItemModel::GetStaticColumnKey(model, integer) : QString::number(integer))
{
}

StandardItemModel::GenericValidation::GenericValidation(QAbstractItemModel* model, bool column, const QString& string)
  : GenericValidation(model, model->property(KeyModelInformation).toMap()
    [column ? KeyColumnInformation : KeyRowInformation].toMap()
    [string].toMap()
    [column ? KeyColumnInformationValidation : KeyRowInformationValidation].toMap())
{
}

bool StandardItemModel::GenericValidation::IsNull() const
{
  return qIsNaN(Minimum)
      && qIsNaN(Maximum)
      && qIsNaN(ExclusiveMinimum)
      && qIsNaN(ExclusiveMaximum)
      && qIsNaN(LocalMinimum)
      && qIsNaN(LocalMaximum)
      && qIsNaN(InputMinimum)
      && qIsNaN(InputMaximum);
}

void StandardItemModel::GenericValidation::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (properties.contains(KeyColumnValidationMinimun))
    Minimum = properties[KeyColumnValidationMinimun].toDouble();
  if (properties.contains(KeyColumnValidationMaximum))
    Maximum = properties[KeyColumnValidationMaximum].toDouble();
  if (properties.contains(KeyColumnValidationExclusiveMinimum))
    ExclusiveMinimum = properties[KeyColumnValidationExclusiveMinimum].toDouble();
  if (properties.contains(KeyColumnValidationExclusiveMaximum))
    ExclusiveMaximum = properties[KeyColumnValidationExclusiveMaximum].toDouble();
  if (properties.contains(KeyColumnValidationLocalMinimum))
    LocalMinimum = properties[KeyColumnValidationLocalMinimum].toDouble();
  if (properties.contains(KeyColumnValidationLocalMaximum))
    LocalMaximum = properties[KeyColumnValidationLocalMaximum].toDouble();
  if (properties.contains(KeyColumnValidationInputMinimum))
    InputMinimum = properties[KeyColumnValidationInputMinimum].toDouble();
  if (properties.contains(KeyColumnValidationInputMaximum))
    InputMaximum = properties[KeyColumnValidationInputMaximum].toDouble();
}

void StandardItemModel::GenericValidation::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyColumnValidationMinimun]          = Minimum;
  properties[KeyColumnValidationMaximum]          = Maximum;
  properties[KeyColumnValidationExclusiveMinimum] = ExclusiveMinimum;
  properties[KeyColumnValidationExclusiveMaximum] = ExclusiveMaximum;
  properties[KeyColumnValidationLocalMinimum]     = LocalMinimum;
  properties[KeyColumnValidationLocalMaximum]     = LocalMaximum;
  properties[KeyColumnValidationInputMinimum]     = InputMinimum;
  properties[KeyColumnValidationInputMaximum]     = InputMaximum;
}

//// Descriptors / Model / Column / Validation
StandardItemModel::ColumnValidation::ColumnValidation()
  : GenericValidation()
{
}

StandardItemModel::ColumnValidation::ColumnValidation(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericValidation(model, properties)
{
}

StandardItemModel::ColumnValidation::ColumnValidation(QAbstractItemModel* model, int integer)
  : GenericValidation(model, true, integer)
{
}

StandardItemModel::ColumnValidation::ColumnValidation(QAbstractItemModel* model, const QString& string)
  : GenericValidation(model, true, string)
{
}

//// Descriptors / Model / Row / Validation
StandardItemModel::RowValidation::RowValidation()
  : GenericValidation()
{
}

StandardItemModel::RowValidation::RowValidation(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericValidation(model, properties)
{
}

StandardItemModel::RowValidation::RowValidation(QAbstractItemModel* model, int integer)
  : GenericValidation(model, false, integer)
{
}

StandardItemModel::RowValidation::RowValidation(QAbstractItemModel* model, const QString& string)
  : GenericValidation(model, false, string)
{
}


//// Descriptors / Model / Column or Row / Format
StandardItemModel::GenericFormat::GenericFormat()
  : Decimals(-1)
  , DecimalsDisplay(-1)
  , Step(std::numeric_limits<double>::quiet_NaN())
  , Scientific(false)
  , IconDisplay(false)
{
}

StandardItemModel::GenericFormat::GenericFormat(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericFormat()
{
  Load(model, properties);
}

StandardItemModel::GenericFormat::GenericFormat(QAbstractItemModel* model, bool column, int integer)
  : GenericFormat(model, column, column ? StandardItemModel::GetStaticColumnKey(model, integer) : QString::number(integer))
{
}

StandardItemModel::GenericFormat::GenericFormat(QAbstractItemModel* model, bool column, const QString& string)
  : GenericFormat(model, model->property(KeyModelInformation).toMap()
    [column ? KeyColumnInformation : KeyRowInformation].toMap()
    [string].toMap()
    [column ? KeyColumnInformationFormat : KeyRowInformationFormat].toMap())
{
}

bool StandardItemModel::GenericFormat::IsNull() const
{
  return Decimals == -1
      && DecimalsDisplay == -1
      && qIsNaN(Step)
      && !Scientific
      && !IconDisplay;
}

void StandardItemModel::GenericFormat::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (properties.contains(KeyFormatDecimals))
    Decimals = properties[KeyFormatDecimals].toInt();
  if (properties.contains(KeyFormatDecimalsDisplay))
    DecimalsDisplay = properties[KeyFormatDecimalsDisplay].toInt();
  if (properties.contains(KeyFormatStep))
    Step = properties[KeyFormatStep].toDouble();
  if (properties.contains(KeyFormatScience))
    Scientific = properties[KeyFormatScience].toBool();
  if (properties.contains(KeyFormatIconDisplay))
    IconDisplay = properties[KeyFormatIconDisplay].toBool();
}

void StandardItemModel::GenericFormat::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyFormatDecimals]         = Decimals;
  properties[KeyFormatDecimalsDisplay]  = DecimalsDisplay;
  properties[KeyFormatStep]             = Step;
  properties[KeyFormatScience]          = Scientific;
  properties[KeyFormatIconDisplay]      = IconDisplay;
}

//// Descriptors / Model / Column / Format
StandardItemModel::ColumnFormat::ColumnFormat()
  : GenericFormat()
{
}

StandardItemModel::ColumnFormat::ColumnFormat(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericFormat(model, properties)
{
}

StandardItemModel::ColumnFormat::ColumnFormat(QAbstractItemModel* model, int integer)
  : GenericFormat(model, true, integer)
{
}

StandardItemModel::ColumnFormat::ColumnFormat(QAbstractItemModel* model, const QString& string)
  : GenericFormat(model, true, string)
{
}

//// Descriptors / Model / Row / Format
StandardItemModel::RowFormat::RowFormat()
  : GenericFormat()
{
}

StandardItemModel::RowFormat::RowFormat(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericFormat(model, properties)
{
}

StandardItemModel::RowFormat::RowFormat(QAbstractItemModel* model, int integer)
  : GenericFormat(model, false, integer)
{
}

StandardItemModel::RowFormat::RowFormat(QAbstractItemModel* model, const QString& string)
  : GenericFormat(model, false, string)
{
}

//// Descriptors / Model / Column / Scheme
StandardItemModel::ColumnScheme::ColumnScheme()
  : Type(QMetaType::UnknownType)
{
}

StandardItemModel::ColumnScheme::ColumnScheme(QAbstractItemModel* model, const QVariantMap& properties)
  : ColumnScheme()
{
  Load(model, properties);
}

StandardItemModel::ColumnScheme::ColumnScheme(QAbstractItemModel* model, int integer)
  : ColumnScheme(model, StandardItemModel::GetStaticColumnKey(model, integer))
{
}

StandardItemModel::ColumnScheme::ColumnScheme(QAbstractItemModel* model, const QString& string)
  : ColumnScheme(model, model->property(KeyModelInformation).toMap()[KeyColumnInformation].toMap()[string].toMap()[KeyScheme].toMap())
{
}

bool StandardItemModel::ColumnScheme::IsNull() const
{
  return Type != QMetaType::UnknownType;
}

void StandardItemModel::ColumnScheme::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  Type         = (QMetaType::Type)properties[KeySchemeType].toInt();
  TypicalValue = properties[KeySchemeTypicalValue];
}

void StandardItemModel::ColumnScheme::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeySchemeType]         = (int)Type;
  properties[KeySchemeTypicalValue] = TypicalValue;
}

//// Descriptors / Model / Column / data connection
StandardItemModel::ColumnDataConnection::ColumnDataConnection()
  : SourceModelPtr(nullptr)
  , SourceIndexKey(-1)
  , SourceIndexName(-1)
  , Select(false)
  , Edit(false)
{
}

StandardItemModel::ColumnDataConnection::ColumnDataConnection(QAbstractItemModel* model, const QVariantMap& properties)
  : ColumnDataConnection()
{
  Load(model, properties);
}

StandardItemModel::ColumnDataConnection::ColumnDataConnection(QAbstractItemModel* model, int integer)
  : ColumnDataConnection(model, StandardItemModel::GetStaticColumnKey(model, integer))
{
}

StandardItemModel::ColumnDataConnection::ColumnDataConnection(QAbstractItemModel* model, const QString& string)
  : ColumnDataConnection(model, model->property(KeyModelInformation).toMap()[KeyColumnInformation].toMap()[string].toMap()[KeyDataConnection].toMap())
{
}

bool StandardItemModel::ColumnDataConnection::IsNull() const
{
  return SourceModelPtr  == nullptr
      || SourceIndexKey  == -1
      || SourceIndexName == -1;
}

void StandardItemModel::ColumnDataConnection::Load(QAbstractItemModel* model, const QVariantMap& properties)
{
  QMap<QString, QString> globalAliases;
  {
    QVariantMap aliases = model->property(KeyModelInformation).toMap()[KeyModelInformationAliases].toMap();
    for (auto a = aliases.begin(); a != aliases.end(); ++a)
    {
      globalAliases[a.key()] = a.value().toString();
    }
  }

  QVariantMap aliases = properties[KeyDataConnectionAliases].toMap();
  for (auto a = aliases.begin(); a != aliases.end(); ++a)
  {
    Aliases[a.key()] = a.value().toString();
  }

  SourceModelPtr = (QAbstractItemModel*)properties[KeyDataConnectionSourceModelPtr].toULongLong();
  GroupModelPtr = (QAbstractItemModel*)properties[KeyDataConnectionGroupModelPtr].toULongLong();

  QVariantMap sourceColumnKeys = properties[KeyDataConnectionSourceColumnKeys].toMap();
  for (auto sk = sourceColumnKeys.begin(); sk != sourceColumnKeys.end(); ++sk)
  {
    int integer = sk.key().toInt();
    QString str = sk.value().toString();
    if (integer != -1 && !str.isEmpty())
    {
      SourceColumnKeys[integer] = str;
      SourceColumnIndexes[str] = integer;
    }
  }

  QVariantMap targetColumnKeys = properties[KeyDataConnectionTargetColumnKeys].toMap();
  for (auto tk = targetColumnKeys.begin(); tk != targetColumnKeys.end(); ++tk)
  {
    int integer = tk.key().toInt();
    QString str = tk.value().toString();
    if (integer != -1 && !str.isEmpty())
    {
      TargetColumnKeys[integer] = str;
      TargetColumnIndexes[str] = integer;
    }
  }

  Select = properties[KeyDataConnectionSelect].toBool();
  Edit   = properties[KeyDataConnectionEdit].toBool();

  ////////////////////////////////////////////////////////////
  // Resolve key index, name index
  {
    if (properties.contains(KeyDataConnectionSourceIndexKey))
      SourceIndexKey = properties[KeyDataConnectionSourceIndexKey].toInt();
    if (properties.contains(KeyDataConnectionSourceIndexName))
      SourceIndexName = properties[KeyDataConnectionSourceIndexName].toInt();

    // Key
    if (SourceIndexKey == -1)
    {
      if (SourceColumnIndexes.contains("Key"))
        SourceIndexKey = SourceColumnIndexes.value("Key");
      else if (SourceColumnIndexes.contains("Connection::Key"))
        SourceIndexKey = SourceColumnIndexes.value("Connection::Key");
      else if (Aliases.contains("Connection::Key") && SourceColumnIndexes.contains(Aliases.value("Connection::Key")))
        SourceIndexKey = SourceColumnIndexes.value(Aliases.value("Connection::Key"));
      else if (globalAliases.contains("Connection::Key") && SourceColumnIndexes.contains(globalAliases.value("Connection::Key")))
        SourceIndexKey = SourceColumnIndexes.value(globalAliases.value("Connection::Key"));
    }

    // Name
    if (SourceIndexName == -1)
    {
      if (SourceColumnIndexes.contains("Name"))
        SourceIndexName = SourceColumnIndexes.value("Name");
      else if (SourceColumnIndexes.contains("Connection::Name"))
        SourceIndexName = SourceColumnIndexes.value("Connection::Name");
      else if (Aliases.contains("Connection::Name") && SourceColumnIndexes.contains(Aliases.value("Connection::Name")))
        SourceIndexName = SourceColumnIndexes.value(Aliases.value("Connection::Name"));
      else if (globalAliases.contains("Connection::Name") && SourceColumnIndexes.contains(globalAliases.value("Connection::Name")))
        SourceIndexName = SourceColumnIndexes.value(globalAliases.value("Connection::Name"));
    }
  }
}

void StandardItemModel::ColumnDataConnection::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  QVariantMap aliases;
  for (auto a = Aliases.begin(); a != Aliases.end(); ++a)
  {
    aliases[a.key()] = a.value();
  }
  properties[KeyDataConnectionAliases] = aliases;

  properties[KeyDataConnectionSourceModelPtr] = (unsigned long long)SourceModelPtr.data();
  properties[KeyDataConnectionGroupModelPtr] = (unsigned long long)GroupModelPtr.data();

  QVariantMap sourceColumnKeys;
  for (auto sk = SourceColumnKeys.begin(); sk != SourceColumnKeys.end(); ++sk)
  {
    sourceColumnKeys[QString::number(sk.key())] = sk.value();
  }
  properties[KeyDataConnectionSourceColumnKeys] = sourceColumnKeys;

  properties[KeyDataConnectionSourceIndexKey]   = SourceIndexKey;
  properties[KeyDataConnectionSourceIndexName]  = SourceIndexName;

  QVariantMap targetColumnKeys;
  for (auto tk = TargetColumnKeys.begin(); tk != TargetColumnKeys.end(); ++tk)
  {
    targetColumnKeys[QString::number(tk.key())] = tk.value();
  }
  properties[KeyDataConnectionTargetColumnKeys] = targetColumnKeys;

  properties[KeyDataConnectionSelect] = Select;
  properties[KeyDataConnectionEdit]   = Edit;
}

//// Descriptors / Model / Column / Predefined values
StandardItemModel::GenericPredefinedValues::GenericPredefinedValues()
  : Editable(false)
{
}

StandardItemModel::GenericPredefinedValues::GenericPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericPredefinedValues()
{
  Load(model, properties);
}

StandardItemModel::GenericPredefinedValues::GenericPredefinedValues(QAbstractItemModel* model, bool column, int integer)
  : GenericPredefinedValues(model, column, column ? StandardItemModel::GetStaticColumnKey(model, integer) : QString::number(integer))
{
}

StandardItemModel::GenericPredefinedValues::GenericPredefinedValues(QAbstractItemModel* model, bool column, const QString& string)
  : GenericPredefinedValues(model, model->property(KeyModelInformation).toMap()
    [column ? KeyColumnInformation : KeyRowInformation].toMap()
    [string].toMap()
    [column ? KeyColumnInformationPredefinedValues : KeyRowInformationPredefinedValues].toMap())
{
}

bool StandardItemModel::GenericPredefinedValues::IsNull() const
{
  return Values.empty();
}

void StandardItemModel::GenericPredefinedValues::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (properties.contains(KeyPredefinedValuesValues))
    Values = properties[KeyPredefinedValuesValues].toMap();
  if (properties.contains(KeyPredefinedValuesOrder))
    Order = properties[KeyPredefinedValuesOrder].toMap();
  if (properties.contains(KeyPredefinedValuesEditable))
    Editable = properties[KeyPredefinedValuesEditable].toBool();
}

void StandardItemModel::GenericPredefinedValues::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyPredefinedValuesValues]   = Values;
  properties[KeyPredefinedValuesOrder]    = Order;
  properties[KeyPredefinedValuesEditable] = Editable;
}

//// Descriptors / Model / Column / Predefined values
StandardItemModel::ColumnPredefinedValues::ColumnPredefinedValues()
  : GenericPredefinedValues()
{
}

StandardItemModel::ColumnPredefinedValues::ColumnPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericPredefinedValues(model, properties)
{
}

StandardItemModel::ColumnPredefinedValues::ColumnPredefinedValues(QAbstractItemModel* model, int integer)
  : GenericPredefinedValues(model, true, integer)
{
}

StandardItemModel::ColumnPredefinedValues::ColumnPredefinedValues(QAbstractItemModel* model, const QString& string)
  : GenericPredefinedValues(model, true, string)
{
}

//// Descriptors / Model / Row / Predefined values
StandardItemModel::RowPredefinedValues::RowPredefinedValues()
  : GenericPredefinedValues()
{
}

StandardItemModel::RowPredefinedValues::RowPredefinedValues(QAbstractItemModel* model, const QVariantMap& properties)
  : GenericPredefinedValues(model, properties)
{
}

StandardItemModel::RowPredefinedValues::RowPredefinedValues(QAbstractItemModel* model, int integer)
  : GenericPredefinedValues(model, false, integer)
{
}

StandardItemModel::RowPredefinedValues::RowPredefinedValues(QAbstractItemModel* model, const QString& string)
  : GenericPredefinedValues(model, false, string)
{
}

//// Descriptors / Model / Column / UOM
StandardItemModel::ColumnUOM::ColumnUOM()
  : Key()
  , Semantics()
  , Quantity(UOM::PhysicalQuantityNone)
  , UnitsStored(UOM::AnyNone)
  , UnitsUI(UOM::AnyNone)
{
}

StandardItemModel::ColumnUOM::ColumnUOM(QAbstractItemModel* model, int integer)
  : ColumnUOM(model, StandardItemModel::GetStaticColumnKey(model, integer))
{
  Key = ColumnKey(model, integer);
  Load(model, QVariantMap());
}

StandardItemModel::ColumnUOM::ColumnUOM(QAbstractItemModel* model, const QString& string)
  : ColumnUOM()
{
  Key = ColumnKey(model, string);
  Load(model, QVariantMap());
}

bool StandardItemModel::ColumnUOM::IsNull() const
{
  return Quantity     == UOM::PhysicalQuantityNone
      && UnitsStored  == UOM::AnyNone
      && UnitsUI      == UOM::AnyNone
      && Semantics.isEmpty();
}

void StandardItemModel::ColumnUOM::Load(QAbstractItemModel* model, const QVariantMap& /*properties*/)
{
  if (!Key.String.isEmpty())
  {
    QVariantMap modelUOM = model->property(UOM::KeyUOM).toMap();
    if (!modelUOM.empty())
    {
      QVariantMap modelUOMQuantities  = modelUOM[UOM::KeyQuantities].toMap();
      QVariantMap modelUOMUnitsStored = modelUOM[UOM::KeyUnitsStored].toMap();
      QVariantMap modelUOMUnitsUI     = modelUOM[UOM::KeyUnitsUI].toMap();
      QVariantMap modelUOMSemantics   = modelUOM[UOM::KeySemantics].toMap();

      if (modelUOMQuantities.contains(Key.String))
        Quantity = (UOM::PhysicalQuantity)modelUOMQuantities[Key.String].toInt();
      if (modelUOMUnitsStored.contains(Key.String))
        UnitsStored = modelUOMUnitsStored[Key.String].toInt();
      if (modelUOMUnitsUI.contains(Key.String))
        UnitsUI = modelUOMUnitsUI[Key.String].toInt();
      if (modelUOMSemantics.contains(Key.String))
        Semantics = modelUOMSemantics[Key.String].toString();

      if (!Semantics.isEmpty())
      {
        if (Quantity == UOM::PhysicalQuantityNone)
          Quantity = UOMSettings::Get().GetQuantityForSemantics(Semantics);
        if (UnitsStored == UOM::AnyNone)
          UnitsStored = UOM::GetSIUnitsForQuantity(Quantity);
        if (UnitsUI == UOM::AnyNone)
          UnitsUI = UOMSettings::Get().GetUiUnitsForSemantics(Semantics);
      }
    }
  }
}

void StandardItemModel::ColumnUOM::Save(QAbstractItemModel* model, QVariantMap& /*properties*/) const
{
  if (!Key.String.isEmpty() && !IsNull())
  {
    QVariantMap modelUOM = model->property(UOM::KeyUOM).toMap();

    QVariantMap modelUOMQuantities  = modelUOM[UOM::KeyQuantities].toMap();
    QVariantMap modelUOMUnitsStored = modelUOM[UOM::KeyUnitsStored].toMap();
    QVariantMap modelUOMUnitsUI     = modelUOM[UOM::KeyUnitsUI].toMap();
    QVariantMap modelUOMSemantics   = modelUOM[UOM::KeySemantics].toMap();

    QStringList keys;
    keys << Key.String;
    if (Key.Integer != -1)
    {
      keys << QString::number(Key.Integer);
    }

    for (const auto& k : keys)
    {
      modelUOMQuantities[k]  = Quantity;
      modelUOMUnitsStored[k] = UnitsStored;
      modelUOMUnitsUI[k]     = UnitsUI;
      modelUOMSemantics[k]   = Semantics;
    }

    modelUOM[UOM::KeyQuantities]    = modelUOMQuantities;
    modelUOM[UOM::KeyUnitsStored]   = modelUOMUnitsStored;
    modelUOM[UOM::KeyUnitsUI]       = modelUOMUnitsUI;
    modelUOM[UOM::KeySemantics]     = modelUOMSemantics;

    model->setProperty(UOM::KeyUOM, modelUOM);

    if (!Key.String.isEmpty())
    {
      if (StandardUOMModel* uomModel = qobject_cast<StandardUOMModel*>(model))
      {
        if (Quantity != UOM::PhysicalQuantityNone)
          uomModel->SetColumnQuantity(Key.String, Quantity);
        if (UnitsStored != UOM::AnyNone)
          uomModel->SetColumnUnitsStored(Key.String, UnitsStored);
        if (UnitsUI != UOM::AnyNone)
          uomModel->SetColumnUnitsUI(Key.String, UnitsUI);
        if (!Semantics.isEmpty())
          uomModel->SetColumnSemantics(Key.String, Semantics);
      }
      else if (AbstractUOMModel* uomModel = qobject_cast<AbstractUOMModel*>(model))
      {
        if (Quantity != UOM::PhysicalQuantityNone)
          uomModel->SetColumnQuantity(Key.String, Quantity);
        if (UnitsStored != UOM::AnyNone)
          uomModel->SetColumnUnitsStored(Key.String, UnitsStored);
        if (UnitsUI != UOM::AnyNone)
          uomModel->SetColumnUnitsUI(Key.String, UnitsUI);
        if (!Semantics.isEmpty())
          uomModel->SetColumnSemantics(Key.String, Semantics);
      }
    }
  }
}

//// Descriptors / Model / Column / Visual
StandardItemModel::ColumnVisual::ColumnVisual()
  : SizeHint()
{
}

StandardItemModel::ColumnVisual::ColumnVisual(QAbstractItemModel* model, int integer)
  : ColumnVisual(model, StandardItemModel::GetStaticColumnKey(model, integer))
{
}

StandardItemModel::ColumnVisual::ColumnVisual(QAbstractItemModel* model, const QString& string)
  : ColumnVisual(model, model->property(KeyModelInformation).toMap()[KeyColumnInformation].toMap()[string].toMap()[KeyColumnInformationVisual].toMap())
{
}

StandardItemModel::ColumnVisual::ColumnVisual(QAbstractItemModel* model, const QVariantMap& properties)
  : ColumnVisual()
{
  Load(model, properties);
}

bool StandardItemModel::ColumnVisual::IsNull() const
{
  return SizeHint.width() < 0
      && SizeHint.height() < 0;
}

void StandardItemModel::ColumnVisual::Load(QAbstractItemModel* /*model*/, const QVariantMap& properties)
{
  if (properties.contains(KeyVisualSizeHintWidth))
    SizeHint.setWidth(properties[KeyVisualSizeHintWidth].toInt());
  if (properties.contains(KeyVisualSizeHintHeight))
    SizeHint.setHeight(properties[KeyVisualSizeHintHeight].toInt());
}

void StandardItemModel::ColumnVisual::Save(QAbstractItemModel* /*model*/, QVariantMap& properties) const
{
  properties[KeyVisualSizeHintWidth]  = SizeHint.width();
  properties[KeyVisualSizeHintHeight] = SizeHint.height();
}

//// Descriptors / Model / Column / Full Column Info
StandardItemModel::ColumnInformation::ColumnInformation()
  : Key()
  , Format()
  , Scheme()
  , DataConnection()
  , Editable(true)
{
}

StandardItemModel::ColumnInformation::ColumnInformation(QAbstractItemModel* model, int integer)
  : ColumnInformation(model, StandardItemModel::GetStaticColumnKey(model, integer))
{
}

StandardItemModel::ColumnInformation::ColumnInformation(QAbstractItemModel* model, const QString& string)
  : ColumnInformation(model, model->property(KeyModelInformation).toMap()[KeyColumnInformation].toMap()[string].toMap(), ColumnKey(model, string))
{
}

StandardItemModel::ColumnInformation::ColumnInformation(QAbstractItemModel* model, const QVariantMap& properties, const ColumnKey& key)
  : ColumnInformation()
{
  Key = key;
  Load(model, properties);
}

bool StandardItemModel::ColumnInformation::IsNull() const
{
  return Key.IsNull();
}

void StandardItemModel::ColumnInformation::Load(QAbstractItemModel* model, const QVariantMap& properties)
{
  Key.Load(model,             properties[KeyColumnInformationKey].toMap());
  Format.Load(model,          properties[KeyColumnInformationFormat].toMap());
  Validation.Load(model,      properties[KeyColumnInformationValidation].toMap());
  Scheme.Load(model,          properties[KeyColumnInformationScheme].toMap());
  DataConnection.Load(model,  properties[KeyColumnInformationDataConnection].toMap());
  Values.Load(model,          properties[KeyColumnInformationPredefinedValues].toMap());
  
  if (properties.contains(KeyColumnInformationEditable))
    Editable = properties[KeyColumnInformationEditable].toBool();
  
  UOM = ColumnUOM(model, Key.String);
  Visual.Load(model, properties[KeyColumnInformationVisual].toMap());
}

void StandardItemModel::ColumnInformation::Save(QAbstractItemModel* model, QVariantMap& properties) const
{
  QVariantMap key;
  Key.Save(model, key);
  properties[KeyColumnInformationKey] = key;

  QVariantMap format;
  Format.Save(model, format);
  properties[KeyColumnInformationFormat] = format;

  QVariantMap validation;
  Validation.Save(model, validation);
  properties[KeyColumnInformationValidation] = validation;

  QVariantMap scheme;
  Scheme.Save(model, scheme);
  properties[KeyColumnInformationScheme] = scheme;

  QVariantMap dataConnection;
  DataConnection.Save(model, dataConnection);
  properties[KeyColumnInformationDataConnection] = dataConnection;

  QVariantMap predefinedValues;
  Values.Save(model, predefinedValues);
  properties[KeyColumnInformationPredefinedValues] = predefinedValues;

  properties[KeyColumnInformationEditable] = Editable;

  QVariantMap dummyUOM;
  UOM.Save(model, dummyUOM);

  QVariantMap visual;
  Visual.Save(model, visual);
  properties[KeyColumnInformationVisual] = visual;
}

void StandardItemModel::ColumnInformation::Save(QAbstractItemModel* model) const
{
  QVariantMap columnInformation;
  Save(model, columnInformation);

  QVariantMap modelInformation = model->property(KeyModelInformation).toMap();
  if (Key.Integer != -1)
    modelInformation[QString::number(Key.Integer)] = columnInformation;
  if (!Key.String.isEmpty())
    modelInformation[Key.String] = columnInformation;
  model->setProperty(KeyColumnInformation, modelInformation);
}

//// Row information (for transposed models)
StandardItemModel::RowInformation::RowInformation()
  : Key(-1)
{
}

StandardItemModel::RowInformation::RowInformation(QAbstractItemModel* model, const QVariantMap& properties, int integer)
  : RowInformation()
{
  Key = integer;
  Load(model, properties);
}

StandardItemModel::RowInformation::RowInformation(QAbstractItemModel* model, int integer)
  : RowInformation(model, model->property(KeyModelInformation).toMap()[KeyRowInformation].toMap()[QString::number(integer)].toMap(), integer)
{
}

bool StandardItemModel::RowInformation::IsNull() const
{
  return Key != -1;
}

void StandardItemModel::RowInformation::Load(QAbstractItemModel* model, const QVariantMap& properties)
{
  if (properties.contains(KeyRowInformationKey))
    Key = properties[KeyRowInformationKey].toInt();
  
  Format.Load(model,      properties[KeyRowInformationFormat].toMap());
  Validation.Load(model,  properties[KeyRowInformationValidation].toMap());
  Values.Load(model,      properties[KeyRowInformationPredefinedValues].toMap());
}

void StandardItemModel::RowInformation::Save(QAbstractItemModel* model, QVariantMap& properties) const
{
  properties[KeyRowInformationKey] = Key;

  QVariantMap format;
  Format.Save(model, format);
  properties[KeyRowInformationFormat] = format;

  QVariantMap validation;
  Validation.Save(model, validation);
  properties[KeyRowInformationValidation] = validation;

  QVariantMap values;
  Values.Save(model, values);
  properties[KeyRowInformationPredefinedValues] = values;
}

////////////////////////////////////////////////////////////
//// Legacy keys resolver (must preceed ModelInformation ctor)
StandardItemModel::ModelInformationLegacyKeysResolver::ModelInformationLegacyKeysResolver(QAbstractItemModel* model)
{
  if (model)
  {
    // fill structured properties here
    StandardItemModel::NormalizeKeys(model);
  }
}

////////////////////////////////////////////////////////////
//// Descriptors / Model / Full Model Info
StandardItemModel::ModelInformation::ModelInformation()
  : ModelInformation(nullptr)
{
}

StandardItemModel::ModelInformation::ModelInformation(QAbstractItemModel* model, const QVariantMap& properties)
  : ModelInformation(nullptr)
{
  Load(model, properties);
}

StandardItemModel::ModelInformation::ModelInformation(QAbstractItemModel* model)
  : ModelInformationLegacyKeysResolver(model)
  , Columns(ColumnKeys)
{
  if (model)
  {
    Load(model);
  }
}

bool StandardItemModel::ModelInformation::IsNull() const
{
  return Columns.empty();
}

void StandardItemModel::ModelInformation::Load(QAbstractItemModel* model, const QVariantMap& properties)
{
  // Column keys
  {
    // Aliases
    QVariantMap aliases = properties[KeyModelInformationAliases].toMap();
    for (auto a = aliases.begin(); a != aliases.end(); ++a)
    {
      Aliases[a.key()]                                  = a.value().toString();
      ColumnKeysStringAliases[a.key()]                  = a.value().toString();
    }

    // Keys
    QVariantMap columnKeys = properties[KeyModelInformationColumnKeys].toMap();
    for (auto ck = columnKeys.begin(); ck != columnKeys.end(); ++ck)
    {
      int integer = ck.key().toInt();
      QString str = ck.value().toString();

      if (integer != -1 && !str.isEmpty())
      {
        ColumnKeys[integer]                               = str;
        ColumnIndexes[str]                                = integer;
        ColumnKeysStringAliases[QString::number(integer)] = str;
        ColumnKeysStringAliases[str]                      = str;
      }
    }
  }

  // Column info
  QVariantMap columnInformation = properties[KeyModelInformationColumnInformation].toMap();
  for (auto column = columnInformation.begin(); column != columnInformation.end(); ++column)
  {
    QString stringKey = ColumnKeysStringAliases.value(column.key(), column.key());
    ColumnInformation ci(model, column.value().toMap(), ColumnKey(ColumnIndexes.value(stringKey, -1), stringKey));
    if (!ci.Key.IsNull())
    {
      if (ci.Key.Integer != -1)
        Columns[QString::number(ci.Key.Integer)] = ci;
      if (!ci.Key.String.isEmpty())
        Columns[ci.Key.String] = ci;
    }
  }

  // Rows
  QVariantMap rowsInformation = properties[KeyModelInformationRowInformation].toMap();

  for (auto row = rowsInformation.begin(); row != rowsInformation.end(); ++row)
  {
    RowInformation rowInformation(model, row.value().toMap(), row.key().toInt());
    if (rowInformation.Key > 0)
    {
      Rows[rowInformation.Key] = rowInformation;
    }
  }

  // Validation
  Validation.Load(model, properties[KeyModelInformationValidation].toMap());

  // Duplucation
  Duplication.Load(model, properties[KeyModelInformationDuplication].toMap());
}

void StandardItemModel::ModelInformation::Load(QAbstractItemModel* model)
{
  return Load(model, model->property(KeyModelInformation).toMap());
}

void StandardItemModel::ModelInformation::Save(QAbstractItemModel* model) const
{
  QVariantMap modelInformation;
  Save(model, modelInformation);
  model->setProperty(KeyModelInformation, modelInformation);
}

void StandardItemModel::ModelInformation::Save(QAbstractItemModel* model, QVariantMap& properties) const
{
  // Aliases
  {
    QVariantMap aliases;
    for (auto a = Aliases.begin(); a != Aliases.end(); ++a)
    {
      aliases[a.key()] = a.value();
    }
    properties[KeyModelInformationAliases] = aliases;
  }

  // Column keys
  {
    QVariantMap columnKeys;
    for (auto ck = ColumnKeys.begin(); ck != ColumnKeys.end(); ++ck)
    {
      columnKeys[QString::number(ck.key())] = ck.value();
    }
    properties[KeyModelInformationColumnKeys] = columnKeys;
  }

  // Columns
  {
    QVariantMap columnInformation;
    for (auto column = Columns.begin(); column != Columns.end(); ++column)
    {
      // Map key
      QString propertyKey;

      // Resolved column key
      ColumnKey resolvedKey;

      // Resolve
      bool ok = true;
      int i = column.key().toInt(&ok);
      if (ok)
      {
        resolvedKey.Integer = column.key().toInt();
        resolvedKey.String  = ColumnKeys.value(resolvedKey.Integer);
      }
      else
      {        
        resolvedKey.String  = column.key();
        auto ck = std::find_if(ColumnKeys.begin(), ColumnKeys.end(), [&resolvedKey](const QString& s) { return resolvedKey.String == s; });
        if (ck != ColumnKeys.end())
        {
          resolvedKey.Integer = ck.key();
        }
      }
      propertyKey = resolvedKey.String.isEmpty() ? QString::number(column.key().toInt()) : resolvedKey.String;

      // Apply resolved key
      if (!resolvedKey.IsNull())
      {
        ColumnKey& k1 = const_cast<ColumnKey&>(column->Key);
        if (k1.IsNull())
          k1 = resolvedKey;
        ColumnKey& k2 = const_cast<ColumnKey&>(column->UOM.Key);
        if (k2.IsNull())
          k2 = resolvedKey;
      }

      // Data
      QVariantMap data;
      column->Save(model, data);

      // Store
      columnInformation[propertyKey] = data;
    }
    properties[KeyModelInformationColumnInformation] = columnInformation;
  }

  // Rows
  {
    QVariantMap rowInformation;
    for (auto row = Rows.begin(); row != Rows.end(); ++row)
    {
      QVariantMap data;
      row.value().Save(model, data);
      rowInformation[QString::number(row.key())] = data;
    }
    properties[KeyModelInformationRowInformation] = rowInformation;
  }

  // Validation
  {
    QVariantMap validation;
    Validation.Save(model, validation);
    properties[KeyModelInformationValidation] = validation;
  }

  // Duplication
  {
    QVariantMap duplication;
    Duplication.Save(model, duplication);
    properties[KeyModelInformationDuplication] = duplication;
  }
}

////////////////////////////////////////////////////////////
//// Standard Item Model Serializable Index Path
StandardItemModel::IndexPath::IndexPath(const QModelIndex& target)
{
  QModelIndex index = target;
  while (index.isValid())
  {
    QString segment = QString("[%1;%2]").arg(index.row()).arg(index.column());
    Segments.prepend(segment);

    index = index.parent();
  } 
}

StandardItemModel::IndexPath::IndexPath(const QStringList& segments)
{
  Segments = segments;
}

StandardItemModel::IndexPath::IndexPath(const QString& path)
{
  Segments = path.split("/");
}

QModelIndex StandardItemModel::IndexPath::GetIndex(QAbstractItemModel* model)
{
  return GetIndex(model, QModelIndex(), Segments);
}

QModelIndex StandardItemModel::IndexPath::GetIndex(QAbstractItemModel* model, const QModelIndex& root, const QStringList& segments)
{
  QStringList childSegments = segments;
  if (childSegments.empty())
    return root;

  QString childsSegment = childSegments.takeFirst();
  childsSegment.remove("[");
  childsSegment.remove("]");
  QStringList childSegmentParts = childsSegment.split(";");
  if (childSegmentParts.size() == 2)
  {
    int childRow = childSegmentParts.first().toInt();
    int childCol = childSegmentParts.last().toInt();
    if (childRow != -1 && childCol != -1)
    {
      QModelIndex child = model->index(childRow, childCol, root);
      if (child.isValid())
      {
        return GetIndex(model, child, childSegments);
      }
    }
  }
  return QModelIndex();
}

QString StandardItemModel::IndexPath::GetPath()
{
  return Segments.join("/");
}

////////////////////////////////////////////////////////////
//// Standard Item Model
StandardItemModel::StandardItemModel(QObject* parent)
  : Inherited(parent)
  , SaveLoadMutex()
{
}

StandardItemModel::StandardItemModel(int rows, int columns, QObject* parent)
  : Inherited(rows, columns, parent)
  , SaveLoadMutex()
{
}

StandardItemModel::~StandardItemModel()
{
}

void StandardItemModel::NormalizeKeys(QAbstractItemModel* /*model*/, bool /*force*/, Qt::ConnectionType /*connection*/)
{
#if 0
  if (connection == Qt::AutoConnection)
  {
    if (model->thread() == QThread::currentThread())
      return NormalizeKeys(model, force, Qt::DirectConnection);
    else
      return NormalizeKeys(model, force, Qt::BlockingQueuedConnection);
  }
  else if (connection == Qt::DirectConnection)
  {
    return NormalizeKeysImpl(model, force);
  }
  else if (connection == Qt::BlockingQueuedConnection)
  {
    bool                    done = false;
    std::mutex              waitMutex;
    std::condition_variable waitCondition;

    QPointer<QAbstractItemModel> instance = model;
    QTimer::singleShot(0, instance, [instance, force, &waitMutex, &waitCondition, &done]() {
      if (instance)
      {
        StandardItemModel::NormalizeKeys(instance, force, Qt::DirectConnection);
      }
      std::lock_guard<std::mutex> lk(waitMutex);
      done = true;
      waitCondition.notify_all();
    });

    std::unique_lock<std::mutex> lk(waitMutex);
    waitCondition.wait(lk, [&done] {
      return done;
    });
  }
#endif // 0
}

void StandardItemModel::NormalizeKeysImpl(QAbstractItemModel* model, bool force)
{
  using namespace Legacy;

  // Normalize source before proxy
  QAbstractProxyModel* proxy = qobject_cast<QAbstractProxyModel*>(model);
  if (proxy)
  {
    if (QAbstractItemModel* source = proxy->sourceModel())
    {
      NormalizeKeys(source, force);
    }
  }

  if ((!model->property("StandardItemModel::KeysNormalized").toBool() || force)
    && !model->property("StandardItemModel::KeysNormalizing").toBool())
  {
    model->setProperty("StandardItemModel::KeysNormalizing", true);

    // Fill structured, reuse existing structured data
    ModelInformation modelInformation(model);

    // 1. Restructure aliases, keys
    {
      // Aliases
      QVariantMap legacyAliases = model->property(LegacyKeyAliases).toMap();
      if (!legacyAliases.empty()
       && modelInformation.Aliases.empty()) // reuse
      {
        for (auto a = legacyAliases.begin(); a != legacyAliases.end(); ++a)
        {
          modelInformation.Aliases[a.key()]                 = a.value().toString();
          modelInformation.ColumnKeysStringAliases[a.key()] = a.value().toString();
        }
      }

      // Keys
      QVariantMap legacyKeys = model->property(LegacyKeyColumnKeys).toMap();
      if (!legacyKeys.empty()
        && modelInformation.ColumnKeys.empty()
        && modelInformation.ColumnIndexes.empty()) // reuse
      {
        for (auto k = legacyKeys.begin(); k != legacyKeys.end(); ++k)
        {
          int integer = k.key().toInt();
          QString str = k.value().toString();

          if (integer != -1 && !str.isEmpty())
          {
            modelInformation.ColumnKeys[integer]                                = str;
            modelInformation.ColumnIndexes[str]                                 = integer;

            modelInformation.ColumnKeysStringAliases[QString::number(integer)]  = str;
            modelInformation.ColumnKeysStringAliases[str]                       = str;
          }
        }
      }
    }

    // 2. Restructure model validation
    {
      QVariantMap legacyValidation = model->property(LegacyKeyValidation).toMap();
      if (!legacyValidation.empty()
        && modelInformation.Validation.IsNull())
      {
        QVariantMap validationData = model->property(KeyModelInformation).toMap()[KeyValidation].toMap(); // reuse
        if (validationData.isEmpty())
        {
          QVariantMap uniquness = validationData[KeyValidationUnique].toMap();

          uniquness[KeyValidationUniqueValue]              = legacyValidation[LegacyKeyValidationUniqueValue];
          uniquness[KeyValidationUniqueValueDefaultName]   = legacyValidation[LegacyKeyValidationUniqueValueDefaultName];
          uniquness[KeyValidationUniqueValueName]          = legacyValidation[LegacyKeyValidationUniqueValueName];

          validationData[KeyValidationUnique] = uniquness;
        }
        modelInformation.Validation.Load(model, validationData);
      }
    }

    // 2. Restructure duplication
    {
      QVariantMap legacyDuplication = model->property(LegacyKeyDuplication).toMap();
      if (!legacyDuplication.empty()
        && modelInformation.Duplication.IsNull())
      {
        QVariantMap duplicationData = model->property(KeyModelInformation).toMap()[KeyDuplication].toMap(); // reuse
        if (duplicationData.isEmpty())
        {
          duplicationData[KeyDuplicationCanDuplicate] = legacyDuplication[LegacyKeyDuplicationCanDuplicate];
          QList<int> legacyDuplicatableList = legacyDuplication[LegacyKeyDuplicationDuplicatable].value<QList<int>>();
          QVariantList duplicatableList;
          for (const auto& d : legacyDuplicatableList)
          {
            duplicatableList << d;
          }
          duplicationData[KeyDuplicationDuplicatable] = duplicatableList;
        }
        modelInformation.Duplication.Load(model, duplicationData);
      }
    }

    // 3. Load columns and store using ColumnKeysStringAliases value
    //    Load rows and store using integer keys
    {
      QMap<QString, ColumnInformation> columnInformationMap;
      QMap<int, RowInformation>        rowInformationMap;

      bool formatIsForRows      = model->property(LegacyKeyFormatIsForRows).toBool();
      bool valuesIsForRows      = model->property(LegacyKeyValuesIsForRows).toBool();
      bool validationIsForRows  = model->property(LegacyKeyValidationIsForRows).toBool();
      
      // Create default constructed column infos
      auto skList = modelInformation.ColumnKeysStringAliases.values(); 
      QSet<QString> stringKeys(skList.begin(), skList.end());
      for (const auto& sk : stringKeys)
      {
        columnInformationMap[sk] = modelInformation.Columns.value(sk); // reuse
      }

      // Create default constructed row infos
      {
        if (formatIsForRows)
        {
          auto stringKeys = model->property(LegacyKeyFormat).toMap().keys();
          for (const auto& sk : stringKeys)
          {
            int ik = sk.toInt();
            rowInformationMap[ik] = modelInformation.Rows.value(ik); // reuse
          }
        }
        if (valuesIsForRows)
        {
          auto stringKeys = model->property(LegacyKeyValues).toMap().keys();
          for (const auto& sk : stringKeys)
          {
            int ik = sk.toInt();
            rowInformationMap[ik] = modelInformation.Rows.value(ik); // reuse
          }
        }
        if (validationIsForRows)
        {
          auto validation = model->property(LegacyKeyValidation).toMap();
          for (auto v = validation.begin(); v != validation.end(); ++v)
          {
            if (v.value().VARIANT_TYPE_ID() == QMetaType::QVariantMap)
            {
              int ik = v.key().toInt();
              rowInformationMap[ik] = modelInformation.Rows.value(ik); // reuse
            }
          }
        }
      }

      // Fill .Key
      {
        for (auto column = columnInformationMap.begin(); column != columnInformationMap.end(); ++column)
        {
          column->Key.String  = column.key();
          column->Key.Integer = modelInformation.ColumnIndexes.value(column.key(), -1);
        }
      }

      // Fill .Validation
      {
        QVariantMap legacyValidation = model->property(LegacyKeyValidation).toMap();
        for (auto v = legacyValidation.begin(); v != legacyValidation.end(); ++v)
        {
          GenericValidation* validation = nullptr;
          if (validationIsForRows)
          {
            int ik = v.key().toInt();
            GenericValidation& rowValidation = rowInformationMap[ik].Validation;
            validation = &rowValidation;
          }
          else
          {
            QString columnKey = modelInformation.ColumnKeysStringAliases.value(v.key(), v.key());
            GenericValidation& columnValidation = columnInformationMap[columnKey].Validation;
            validation = &columnValidation;
          }

          if (validation->IsNull()) // reuse
          {
            QVariantMap legacyValidationData = v.value().toMap();
            QVariantMap validationData;

            if (legacyValidationData.contains(LegacyKeyFormatMinimun))
              validationData[KeyColumnValidationMinimun] = legacyValidationData[LegacyKeyFormatMinimun];
            if (legacyValidationData.contains(LegacyKeyFormatMaximum))
              validationData[KeyColumnValidationMaximum] = legacyValidationData[LegacyKeyFormatMaximum];

            if (legacyValidationData.contains(LegacyKeyFormatExclusiveMinimum))
              validationData[KeyColumnValidationExclusiveMinimum] = legacyValidationData[LegacyKeyFormatExclusiveMinimum];
            if (legacyValidationData.contains(LegacyKeyFormatExclusiveMaximum))
              validationData[KeyColumnValidationExclusiveMaximum] = legacyValidationData[LegacyKeyFormatExclusiveMaximum];

            if (legacyValidationData.contains(LegacyKeyFormatLocalMinimum))
              validationData[KeyColumnValidationLocalMinimum] = legacyValidationData[LegacyKeyFormatLocalMinimum];
            if (legacyValidationData.contains(LegacyKeyFormatLocalMaximum))
              validationData[KeyColumnValidationLocalMaximum] = legacyValidationData[LegacyKeyFormatLocalMaximum];

            validation->Load(model, validationData);
          }
        }
      }

      // Fill .Format
      {
        QVariantMap legacyFormat = model->property(LegacyKeyFormat).toMap();
        for (auto f = legacyFormat.begin(); f != legacyFormat.end(); ++f)
        {
          GenericFormat* format = nullptr;
          if (formatIsForRows)
          {
            int ik = f.key().toInt();
            GenericFormat& rowFormat = rowInformationMap[ik].Format;
            format = &rowFormat;
          }
          else
          {
            QString columnKey = modelInformation.ColumnKeysStringAliases.value(f.key(), f.key());
            GenericFormat& columnFormat = columnInformationMap[columnKey].Format;
            format = &columnFormat;
          }

          if (format->IsNull()) // reuse
          {
            QVariantMap legacyFormatData = f.value().toMap();
            QVariantMap formatData;

            if (legacyFormatData.contains(LegacyKeyFormatDecimals))
              formatData[KeyFormatDecimals] = legacyFormatData[LegacyKeyFormatDecimals];
            if (legacyFormatData.contains(LegacyKeyFormatDecimalsDisplay))
              formatData[KeyFormatDecimalsDisplay] = legacyFormatData[LegacyKeyFormatDecimalsDisplay];
            if (legacyFormatData.contains(LegacyKeyFormatStep))
              formatData[KeyFormatStep] = legacyFormatData[LegacyKeyFormatStep];            
            if (legacyFormatData.contains(LegacyKeyFormatScience))
              formatData[KeyFormatScience] = true;
            if (legacyFormatData.contains(LegacyKeyFormatIconDisplay))
              formatData[KeyFormatIconDisplay] = legacyFormatData[LegacyKeyFormatIconDisplay];

            format->Load(model, formatData);
          }
        }
      }

      // Skip .Scheme (has no legacy layout)

      // Fill .DataConnection
      {
        QVariantMap legacyDataConnections = model->property(LegacyKeyDataConnections).toMap();
        for (auto dc = legacyDataConnections.begin(); dc != legacyDataConnections.end(); ++dc)
        {
          QString columnKey = modelInformation.ColumnKeysStringAliases.value(dc.key(), dc.key());
          if (columnInformationMap[columnKey].DataConnection.IsNull())
          {
            QVariantMap legacyDataConnectionData = dc.value().toMap();
            if (!legacyDataConnectionData.isEmpty())
            {
              QVariantMap dataConnectionData;
              dataConnectionData[KeyDataConnectionSourceModelPtr]     = legacyDataConnectionData[LegacyKeyDataConnectionsSourceModelPtr];
              dataConnectionData[KeyDataConnectionSourceColumnKeys]   = legacyDataConnectionData[LegacyKeyDataConnectionsSourceColumnKeys];
              dataConnectionData[KeyDataConnectionTargetColumnKeys]   = legacyDataConnectionData[LegacyKeyDataConnectionsTargetColumnKeys];
              dataConnectionData[KeyDataConnectionSelect]             = legacyDataConnectionData[LegacyKeyDataConnectionsSelect];
              dataConnectionData[KeyDataConnectionEdit]               = legacyDataConnectionData[LegacyKeyDataConnectionsEdit];

              columnInformationMap[columnKey].DataConnection.Load(model, dataConnectionData);
            }
          }
        }
      }

      // Fill .Values
      {
        // .Editbale
        QVariant valuesEditable = model->property(LegacyKeyValuesEditable);

        // .Values
        {
          QVariantMap legacyValues = model->property(LegacyKeyValues).toMap();
          for (auto v = legacyValues.begin(); v != legacyValues.end(); ++v)
          {
            GenericPredefinedValues* values = nullptr;
            if (valuesIsForRows)
            {
              int ik = v.key().toInt();
              GenericPredefinedValues& rowValues = rowInformationMap[ik].Values;
              values = &rowValues;
            }
            else
            {
              QString columnKey = modelInformation.ColumnKeysStringAliases.value(v.key(), v.key());
              GenericPredefinedValues& columnValues = columnInformationMap[columnKey].Values;
              values = &columnValues;
            }

            if (values->IsNull()) // reuse
            {
              values->Values = v.value().toMap();
              if (valuesEditable.VARIANT_TYPE_ID() == QMetaType::Bool)
              {
                values->Editable = valuesEditable.toBool();
              }
            }
          }
        }

        // .Order
        {
          QVariantMap legacyValuesOrder = model->property(LegacyKeyValuesOrder).toMap();
          for (auto v = legacyValuesOrder.begin(); v != legacyValuesOrder.end(); ++v)
          {
            QString columnKey = modelInformation.ColumnKeysStringAliases.value(v.key(), v.key());
            if (columnInformationMap[columnKey].Values.Order.empty()) // reuse
            {
              columnInformationMap[columnKey].Values.Order = v.value().toMap();
              if (valuesEditable.VARIANT_TYPE_ID() == QMetaType::Bool)
              {
                columnInformationMap[columnKey].Values.Editable = valuesEditable.toBool();
              }
            }
          }
        }
      }

      // Store columns
      for (auto column = columnInformationMap.begin(); column != columnInformationMap.end(); ++column)
      {
        // Key may be dynamic
        if (column->Key.String.isEmpty())
        {
          column->Key.String = column.key();
        }

        // Store by string key
        if (!column->Key.String.isEmpty())
        {
          modelInformation.Columns[column->Key.String] = column.value();
        }

        // Store by ineteger key either
        if (column->Key.Integer != -1)
        {
          modelInformation.Columns[QString::number(column->Key.Integer)] = column.value();
        }
      }

      // Store rows
      for (auto row = rowInformationMap.begin(); row != rowInformationMap.end(); ++row)
      {
        modelInformation.Rows[row.key()] = row.value();
      }
    }

    // Done, store
    modelInformation.Save(model);

#if 0
    {
      QJsonDocument doc;
      QJsonObject root;
      root[KeyModelInformation] = QJsonObject::fromVariantMap(model->property(KeyModelInformation).toMap());
      root[UOM::KeyUOM] = QJsonObject::fromVariantMap(model->property(UOM::KeyUOM).toMap());
      doc.setObject(root);

      {
        QString fn = "C:/git/gui/!tmp/modelinformation-" + QString(model->metaObject()->className()).replace(":", "-") + ".json";
        QFile f(fn);
        f.open(QIODevice::WriteOnly);
        f.write(doc.toJson());
      }
    }
#endif //
    
    // Mark as mormalized
    model->setProperty("StandardItemModel::KeysNormalizing", false);
    model->setProperty("StandardItemModel::KeysNormalized", true);
  }
}

////////////////////////////////////////////////////////////
//// Static 
StandardItemModel::ModelInformation StandardItemModel::GetModelInformation(QAbstractItemModel* model)
{
  return ModelInformation(model);
}

void StandardItemModel::SetModelInformation(QAbstractItemModel* model, const ModelInformation& modelInformation)
{
  modelInformation.Save(model);

  NotifyModelInformationChanged(model);
}

StandardItemModel::ColumnInformation StandardItemModel::GetColumnInformation(QAbstractItemModel* model, const int& index)
{
  return ColumnInformation(model, index);
}

StandardItemModel::ColumnInformation StandardItemModel::GetColumnInformation(QAbstractItemModel* model, const QString& key)
{
  return ColumnInformation(model, key);
}

void StandardItemModel::SetColumnInformation(QAbstractItemModel* model, const ColumnInformation& columnInformation)
{
  ModelInformation mi(model);
  if (columnInformation.Key.Integer != -1)
    mi.Columns[QString::number(columnInformation.Key.Integer)] = columnInformation;
  if (!columnInformation.Key.String.isEmpty())
    mi.Columns[columnInformation.Key.String] = columnInformation;
  mi.Save(model);

  NotifyModelInformationChanged(model);
}

void StandardItemModel::SetColumnInformation(QAbstractItemModel* model, const int& index, const ColumnInformation& columnInformation)
{
  ColumnInformation ci = columnInformation;
  ci.Key = ColumnKey(model, index);
  ci.UOM.Key = ci.Key;
  return SetColumnInformation(model, ci);
}

void StandardItemModel::SetColumnInformation(QAbstractItemModel* model, const QString& key, const ColumnInformation& columnInformation)
{
  ColumnInformation ci = columnInformation;
  ci.Key = ColumnKey(model, key);
  ci.UOM.Key = ci.Key;
  return SetColumnInformation(model, ci);
}

//// Static / Column keys
QVariantMap StandardItemModel::GetColumnKeys(QAbstractItemModel* model)
{
  auto i = model->property(KeyModelInformation);
  if (i.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
  {
    const auto& info = *reinterpret_cast<const QVariantMap*>(&i);
    if (info.contains(KeyModelInformationColumnKeys))
    {
      const auto& k = info[KeyModelInformationColumnKeys];
      if (k.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      {
        const auto& keys = *reinterpret_cast<const QVariantMap*>(&k);
        return std::move(keys);
      }
    }
  }
  return std::move(QVariantMap());
}

void StandardItemModel::SetColumnKeys(QAbstractItemModel* model, const QVariantMap& keys)
{
  QVariantMap mi = model->property(KeyModelInformation).toMap();
  mi[KeyModelInformationColumnKeys] = keys;
  model->setProperty(KeyModelInformation, mi);
}

void StandardItemModel::SetColumnKeys(QAbstractItemModel* model, const QMap<int, QString>& keys)
{
  QVariantMap vkeys;
  for (auto k = keys.begin(); k != keys.end(); ++k)
  {
    vkeys[QString::number(k.key())] = k.value();
  }
  SetColumnKeys(model, vkeys);
}

//// Static / Specific column key
void StandardItemModel::SetColumnKey(QAbstractItemModel* model, const int& column, const QString& key)
{
  auto vkeys = GetColumnKeys(model);
  vkeys[QString::number(column)] = key;
  SetColumnKeys(model, vkeys);
}

int StandardItemModel::GetColumnIndex(QAbstractItemModel* model, const QString& key)
{
  auto vkeys = GetColumnKeys(model);
  auto ck = std::find_if(vkeys.begin(), vkeys.end(), [&key](const QVariant& v) { return key == v.toString(); });
  if (ck != vkeys.end())
    return ck.key().toInt();

  return -1;
}

QString StandardItemModel::GetStaticColumnKey(QAbstractItemModel* model, const int& key)
{
  return std::move(GetColumnKeys(model)[QString::number(key)].toString());
}

QString StandardItemModel::GetDynamicColumnKey(QAbstractItemModel* model, const QModelIndex& modelIndex)
{
  return model->data(modelIndex, UserRoleColumnKey).toString();
}

void StandardItemModel::NotifyModelInformationChanged(QAbstractItemModel* model)
{
  if (StandardItemModel* knownModel = qobject_cast<StandardItemModel*>(model))
  {
    knownModel->NotifyModelInformationChanged();
  }
  else if (AbstractUOMModel* knownModel = qobject_cast<AbstractUOMModel*>(model))
  {
    knownModel->NotifyModelInformationChanged();
  }
}

int StandardItemModel::GetColumnIndex(const QVariantMap& columnKeys, const QString& key)
{
  auto ck = std::find_if(columnKeys.begin(), columnKeys.end(), [&key](const QVariant& v) { return key == v.toString(); });
  if (ck != columnKeys.end())
    return ck.key().toInt();

  return -1;
}

QString StandardItemModel::GetColumnKey(const QVariantMap& columnKeys, const int& key)
{
  return columnKeys[QString::number(key)].toString();
}

////////////////////////////////////////////////////////////
//// Model information
StandardItemModel::ModelInformation StandardItemModel::GetModelInformation() const
{
  return StandardItemModel::GetModelInformation(const_cast<StandardItemModel*>(this));
}

void StandardItemModel::SetModelInformation(const ModelInformation& modelInformation)
{
  StandardItemModel::SetModelInformation(this, modelInformation);
}

////////////////////////////////////////////////////////////
//// Column information
StandardItemModel::ColumnInformation StandardItemModel::GetColumnInformation(const int& index) const
{
  return ColumnInformation(const_cast<StandardItemModel*>(this), index);
}

StandardItemModel::ColumnInformation StandardItemModel::GetColumnInformation(const QString& key) const
{
  return ColumnInformation(const_cast<StandardItemModel*>(this), key);
}

//// Generics
StandardItemModel::GenericValidation StandardItemModel::GetColumnValidation(const QModelIndex& index) const
{
  ModelInformation modelInformation(const_cast<StandardItemModel*>(this));
  if (!index.parent().isValid())
  {
    auto r = modelInformation.Rows.find(index.row());
    if (r != modelInformation.Rows.end())
    {
      if (!r->Values.IsNull())
        return r->Validation;
    }
  }
  return modelInformation.Columns[GetColumnKey(index)].Validation;
}

StandardItemModel::GenericPredefinedValues StandardItemModel::GetColumnPredefinedValues(const QModelIndex& index) const
{
  ModelInformation modelInformation(const_cast<StandardItemModel*>(this));
  if (!index.parent().isValid())
  {
    auto r = modelInformation.Rows.find(index.row());
    if (r != modelInformation.Rows.end())
    {
      if (!r->Values.IsNull())
        return r->Values;
    }
  }
  return modelInformation.Columns[GetColumnKey(index)].Values;
}

////////////////////////////////////////////////////////////
//// Column keys
QVariantMap StandardItemModel::GetColumnKeys() const
{
  return GetColumnKeys(const_cast<StandardItemModel*>(this));
}

QStringList StandardItemModel::GetStaticColumnKeys() const
{
  QStringList result;
  auto i = property(KeyModelInformation);
  if (i.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
  {
    const auto& info = *reinterpret_cast<const QVariantMap*>(&i);
    if (info.contains(KeyModelInformationColumnKeys))
    {
      const auto& k = info[KeyModelInformationColumnKeys];
      if (k.VARIANT_TYPE_ID() == QMetaType::QVariantMap)
      {
        const auto& columnKeys = *reinterpret_cast<const QVariantMap*>(&k);
        result.reserve(columnKeys.size() * 2);
        result << columnKeys.keys();
        for (const auto& key : columnKeys)
        {
          result << key.toString();
        }
      }
    }
  }
  return std::move(result);
}

void StandardItemModel::SetColumnKeys(const QVariantMap& keys)
{
  SetColumnKeys(this, keys);
}

void StandardItemModel::SetColumnKeys(const QMap<int, QString>& keys)
{
  SetColumnKeys(this, keys);
}

void StandardItemModel::SetColumnKey(const int& column, const QString& key)
{
  SetColumnKey(this, column, key);
}

int StandardItemModel::GetColumnIndex(const QString& key) const
{
  return GetColumnIndex(const_cast<StandardItemModel*>(this), key);
}

QString StandardItemModel::GetColumnKey(const QModelIndex& modelIndex) const
{
  if (!modelIndex.isValid())
    return QString();

  QString dynamicKey = GetDynamicColumnKey(modelIndex);
  if (!dynamicKey.isEmpty())
    return dynamicKey;

  return std::move(GetStaticColumnKey(modelIndex));
}

QString StandardItemModel::GetStaticColumnKey(const int& key) const
{
  return std::move(GetColumnKeys()[QString::number(key)].toString());
}

QString StandardItemModel::GetStaticColumnKey(const QModelIndex& modelIndex) const
{
  if (modelIndex.parent().isValid())
    return QString();
  return std::move(GetStaticColumnKey(modelIndex.column()));
}

QString StandardItemModel::GetDynamicColumnKey(const QModelIndex& modelIndex) const
{
  return std::move(data(modelIndex, UserRoleColumnKey).toString());
}

//// Set column editable
void StandardItemModel::SetColumnEditable(int column, bool edit)
{
  ColumnInformation ci(this, column);
  ci.Editable = edit;
  ci.Save(this);
}

void StandardItemModel::SetColumnEditable(const QString& key, bool edit)
{
  ColumnInformation ci(this, key);
  ci.Editable = edit;
  ci.Save(this);
}

//// Column format
void StandardItemModel::SetColumnFormat(const QString& key, const QVariantMap& format)
{
  ColumnInformation ci(this, key);
  ci.Format = ColumnFormat(this, format);
  ci.Save(this);
}

void StandardItemModel::SetColumnFormat(int key, const QVariantMap& format)
{
  ColumnInformation ci(this, key);
  ci.Format = ColumnFormat(this, format);
  ci.Save(this);
}

void StandardItemModel::SetColumnFormat(const QString& key, const ColumnFormat& format)
{
  ColumnInformation ci(this, key);
  ci.Format = format;
  ci.Save(this);
}

void StandardItemModel::SetColumnFormat(int key, const ColumnFormat& format)
{
  ColumnInformation ci(this, key);
  ci.Format = format;
  ci.Save(this);
}

//// Model validation 
StandardItemModel::ModelValidation StandardItemModel::GetValidation() const
{
  return ModelValidation(const_cast<StandardItemModel*>(this));
}

void StandardItemModel::SetValidation(const ModelValidation& validation)
{
  validation.Save(this);
}

//// Model record duplication rules
StandardItemModel::ModelDuplication StandardItemModel::GetDuplication() const
{
  return ModelDuplication(const_cast<StandardItemModel*>(this));
}

void StandardItemModel::SetDuplication(const ModelDuplication& duplication) 
{
  duplication.Save(this);
}

void StandardItemModel::NormalizeKeys(bool force)
{
  NormalizeKeys(this, force);
}

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING
//// Invoke Save / Load with specified connection type
void StandardItemModel::Load(const rapidjson::Value& json, Qt::ConnectionType connection)
{
  if (connection == Qt::AutoConnection)
  {
    bool modelThread = thread() == QThread::currentThread();
    connection = modelThread ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    return Load(json, connection);
  }
  if (connection == Qt::DirectConnection)
  {
    QMutexLocker lk(&SaveLoadMutex);
    NormalizeKeys(this, false);
    if (GetColumnKeys().isEmpty())
      qDebug() << "StandardItemModel::Load" << metaObject()->className() << "w/o ColumnKeys";
    SetLoading(true);
    Load(json);
    SetLoading(false);
  }
  else
  {
    QMutexLocker lk(&SaveLoadMutex);
    QMetaObject::invokeMethod(this, "NormalizeKeys",  connection, Q_ARG(bool, false));
    if (GetColumnKeys().isEmpty())
      qDebug() << "StandardItemModel::Load" << metaObject()->className() << "w/o ColumnKeys";
    QMetaObject::invokeMethod(this, "SetLoading",     connection, Q_ARG(bool, true));
    QMetaObject::invokeMethod(this, "Load",           connection, Q_ARG(const rapidjson::Value&, json));
    QMetaObject::invokeMethod(this, "SetLoading",     connection, Q_ARG(bool, false));
  }
}

void StandardItemModel::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator, Qt::ConnectionType connection) const
{
  if (connection == Qt::AutoConnection)
  {
    bool modelThread = thread() == QThread::currentThread();
    connection = modelThread ? Qt::DirectConnection : Qt::BlockingQueuedConnection;
    return Save(json, allocator, connection);
  }
  if (connection == Qt::DirectConnection)
  {
    QMutexLocker lk(&SaveLoadMutex);
    NormalizeKeys(const_cast<StandardItemModel*>(this), false);
    if (GetColumnKeys().isEmpty())
      qDebug() << "StandardItemModel::Save" << metaObject()->className() << "w/o ColumnKeys";
    Save(json, allocator);
  }
  else if (connection == Qt::BlockingQueuedConnection)
  {
    QMutexLocker lk(&SaveLoadMutex);
    QMetaObject::invokeMethod(const_cast<StandardItemModel*>(this), "NormalizeKeys", connection, Q_ARG(bool, false));
    if (GetColumnKeys().isEmpty())
      qDebug() << "StandardItemModel::Save" << metaObject()->className() << "w/o ColumnKeys";
    QMetaObject::invokeMethod(const_cast<StandardItemModel*>(this), "Save",          connection, Q_ARG(rapidjson::Value&, json), Q_ARG(rapidjson::Document::AllocatorType&, allocator));
  }
}

//// Save / Load
void StandardItemModel::Load(const rapidjson::Value& json)
{
  // Remove all
  removeRows(0, rowCount());
  removeColumns(0, columnCount());

  // Resize
  if (json.HasMember("columnCount")
   && json.HasMember("rowCount"))
  {
    const int columnCount = json["columnCount"].GetIntSafe();
    const int rowCount    = json["rowCount"].GetIntSafe();
    
    setColumnCount(columnCount);
    setRowCount(rowCount);
  }

  //// Create data mapping
  QMap<int, int> columnsMapping;
  {
    // json backward map
    QMap<QString, int> sourceDataColumnKeysVariantsBackward;
    if (json.HasMember("columnKeys"))
    {
      auto& sourceDataColumnKeysVariants = json["columnKeys"];
      if (sourceDataColumnKeysVariants.IsObject())
      {
        auto sckv = sourceDataColumnKeysVariants.MemberBegin();
        while (sckv != sourceDataColumnKeysVariants.MemberEnd())
        {
          sourceDataColumnKeysVariantsBackward[sckv->value.GetString()] = QString(sckv->name.GetString()).toInt();
          ++sckv;
        }
      }
    }

    // model backward map
    auto modelColumnKeys = GetColumnKeys();
    QMap<QString, int> modelColumnKeysBackward;
    {
      QVariantMap::iterator mck = modelColumnKeys.begin();
      while (mck != modelColumnKeys.end())
      {
        modelColumnKeysBackward[mck.value().toString()] = mck.key().toInt();
        ++mck;
      }
    }

    // fill route
    auto sckb = sourceDataColumnKeysVariantsBackward.begin();
    while (sckb != sourceDataColumnKeysVariantsBackward.end())
    {
      int sourceDataColumnIndex = sckb.value();
      auto modelColumnIndex = modelColumnKeysBackward.find(sckb.key());
      if (modelColumnIndex != modelColumnKeysBackward.end())
      {
        columnsMapping[sourceDataColumnIndex] = modelColumnIndex.value();
      }
      ++sckb;
    }
  }

  // None of the indexes matched, accept all
  if (columnsMapping.empty())
  {
    QVariantMap modelColumnKeys = GetColumnKeys();
    while (columnsMapping.size() < modelColumnKeys.size())
    {
      int k = columnsMapping.size();
      columnsMapping[k] = k;
    }
  }

  // Advance to data
  if (json.HasMember("data"))
  {
    auto& data = json["data"];
    if (data.IsArray())
    {
      for (int sourceDataRow = 0; sourceDataRow < rowCount() && sourceDataRow < data.Size(); ++sourceDataRow)
      {
        auto& row = data[sourceDataRow];
        if (row.IsArray())
        {
          for (int sourceDataColumn = 0; sourceDataColumn < columnCount() && sourceDataColumn < row.Size(); ++sourceDataColumn)
          {
            // Get column index from mapping
            int modelColumnIndex = -1;
            int modelRowIndex    = sourceDataRow;
            auto mapping         = columnsMapping.find(sourceDataColumn);
            if (mapping != columnsMapping.end())
            {
              modelColumnIndex = mapping.value();
            }

            // Default to source index
            if (modelColumnIndex == -1)
              modelColumnIndex = sourceDataColumn;
            
            Load(index(modelRowIndex, modelColumnIndex), row[sourceDataColumn]);
          }
        }
      }
    }
  }

  if (auxiliaryLoader) {
    auxiliaryLoader(json);
  }
}

void StandardItemModel::Load(const QModelIndex& idx, const rapidjson::Value& json)
{
  if (idx.isValid())
  {
    //// Item may absent, create
    auto item = itemFromIndex(idx);
    if (!item)
    {
      auto parentItem = itemFromIndex(idx.parent());
      if (parentItem)
      {
        StandardItem* si = new StandardItem;
        parentItem->setChild(idx.row(), idx.column(), si);
      }
    }

    //// Handle value    
    if (json.IsObject() && json.HasMember("value"))
    {
      auto& value = json["value"];
      if (value.IsObject())
      {
        for (auto v = value.MemberBegin(); v != value.MemberEnd(); ++v)
        {
          Qt::ItemDataRole role = (Qt::ItemDataRole)QString(v->name.GetString()).toInt();
          QVariant variant = RapidJsonToQVariant(v->value);
          setData(idx, variant, role);
          switch (role)
          {
          case Qt::CheckStateRole:
            {
              setData(idx, (Qt::CheckState)variant.toInt(), role);
              setData(idx, (Qt::CheckState)variant.toInt(), Qt::EditRole);
            }
            break;
          case StandardItemModel::UserRoleIconAsString:
            {
              setData(idx, variant.toString(), StandardItemModel::UserRoleIconAsString);
            }
            break;
          case StandardItemModel::UserRoleKeyForValue: // <- already handled
            break;
          default:
            break;
          }
        }
      }
      else
      {
        QVariant variant = RapidJsonToQVariant(value);
        setData(idx, variant, Qt::EditRole);
      }
    }
    else if (!json.IsObject() && !json.IsArray())
    {
      QVariant variant = RapidJsonToQVariant(json);
      setData(idx, variant, Qt::EditRole);
    }

    //// Reenter
    if (json.IsObject())
    {
      if (json.HasMember("columnCount")
       && json.HasMember("rowCount")
       && json.HasMember("data"))
      {
        const int columnCount = json["columnCount"].GetIntSafe();
        const int rowCount    = json["rowCount"].GetIntSafe();

        StandardItem* parentItem = itemFromIndex(idx);
        if (parentItem)
        {
          if (columnCount)
            parentItem->setColumnCount(columnCount);
          if (rowCount)
            parentItem->setRowCount(rowCount);
        }
        else
        {
          if (columnCount)
            setColumnCount(columnCount);
          if (rowCount)
            setRowCount(rowCount);
        }

        auto& data = json["data"];
        for (int r = 0; r < rowCount; ++r)
        {
          auto& row = data[r];
          LoadRow(idx, r, row);
        }
      }
    }
  }
}

void StandardItemModel::LoadRow(const QModelIndex& parent, int r, const rapidjson::Value& json)
{
  if (json.IsArray())
  {
    int columns = columnCount(parent);
    for (int c = 0; c < columns && c < json.Size(); ++c)
    {
      QModelIndex mi = index(r, c, parent);
      if (!mi.isValid())
      {
        StandardItem* parentItem = itemFromIndex(parent);
        if (parentItem)
        {
          StandardItem* si = new StandardItem;
          parentItem->setChild(r, c, si);
          mi = index(r, c, parent);
        }
      }
      if (mi.isValid())
      {
        Load(mi, json[c]);
      }
    }
  }
}

void StandardItemModel::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const
{
  json.SetObject();

  // Save data
  Save(QModelIndex(), json, allocator);

  // Save header data
  rapidjson::Value headerDataArray;
  headerDataArray.SetArray();
  for (int c = 0; c < columnCount(); c++)
  {
    headerDataArray.PushBack(rapidjson::Value(headerData(c, Qt::Horizontal, Qt::DisplayRole).toString().toStdString(), allocator), allocator);
  }
  json.AddMember(rapidjson::Value("headerData", allocator), headerDataArray, allocator);

  // Column keys
  QVariantMap columnKeys = GetColumnKeys();
  if (!columnKeys.isEmpty())
  {
    rapidjson::Value columnKeysObject;
    columnKeysObject.SetObject();
    for (int c = 0; c < columnCount(); ++c)
    {
      columnKeysObject.AddMember(rapidjson::Value(QString::number(c).toStdString(), allocator),
                                 rapidjson::Value(columnKeys.value(QString::number(c)).toString().toStdString(), allocator),
                                 allocator);
    }
    json.AddMember(rapidjson::Value("columnKeys", allocator), columnKeysObject, allocator);
  }

  if (auxiliarySaver) {
    auxiliarySaver(json, allocator);
  }
}

void StandardItemModel::Save(const QModelIndex& idx, rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) const
{
  // Edit role value
  rapidjson::Value valueEditRole = QVariantToRapidJson(data(idx, Qt::EditRole), allocator);
  json.AddMember(rapidjson::Value("value", allocator), valueEditRole, allocator);
  
  // Columns, rows
  json.AddMember(rapidjson::Value("rowCount", allocator), rapidjson::Value(rowCount(idx)), allocator);
  json.AddMember(rapidjson::Value("columnCount", allocator), rapidjson::Value(columnCount(idx)), allocator);

  // Data
  rapidjson::Value data;
  data.SetArray();

  int rc = rowCount(idx);
  data.Reserve(rc, allocator);
  for (int r = 0; r < rc; ++r)
  {
    rapidjson::Value row;
    row.SetArray();

    SaveRow(idx, r, row, allocator);
    data.PushBack(row, allocator);
  }
  json.AddMember(rapidjson::Value("data", allocator), data, allocator);
}

void StandardItemModel::SetAuxiliarySaver(std::function<void(rapidjson::Value&, rapidjson::Document::AllocatorType&)> f)
{
  Q_ASSERT(!auxiliarySaver);
  auxiliarySaver = f;
}

void StandardItemModel::SetAuxiliaryLoader(std::function<void(const rapidjson::Value&)> f)
{
  Q_ASSERT(!auxiliaryLoader);
  auxiliaryLoader = f;
}

std::function<void(rapidjson::Value&, rapidjson::Document::AllocatorType&)> StandardItemModel::AuxiliarySaver() const
{
  return auxiliarySaver;
}

std::function<void(const rapidjson::Value&)> StandardItemModel::AuxiliaryLoader() const
{
  return auxiliaryLoader;
}

void StandardItemModel::SaveRow(const QModelIndex& idx, int r, rapidjson::Value& row, rapidjson::Document::AllocatorType& allocator) const
{
  int cc = columnCount(idx);
  row.Reserve(cc, allocator);
  for (int c = 0; c < cc; ++c)
  {
    QModelIndex mi = index(r, c, idx);
    if (rowCount(mi) || columnCount(mi))
    {
      rapidjson::Value object;
      object.SetObject();

      Save(mi, object, allocator);

      row.PushBack(object, allocator);
    }
    else
    {
      QVariantMap dataMap;
      FillItemDataMap(mi, dataMap);

      if (dataMap.size() > 1)
      {
        rapidjson::Value itemObject;
        itemObject.SetObject();

        rapidjson::Value valueObject;
        valueObject.SetObject();

        QVariantMap::iterator d = dataMap.begin();
        while (d != dataMap.end())
        {
          rapidjson::Value v = QVariantToRapidJson(d.value(), allocator);
          valueObject.AddMember(rapidjson::Value(d.key().toStdString(), allocator), v, allocator);
          ++d;
        }

        itemObject.AddMember(rapidjson::Value("value", allocator), valueObject, allocator);
        row.PushBack(itemObject, allocator);
      }
      else
      {
        rapidjson::Value v = QVariantToRapidJson(dataMap.value(QString::number(Qt::EditRole)), allocator);
        row.PushBack(v, allocator);
      }
    }
  }
}
#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

void StandardItemModel::FillItemDataMap(const QModelIndex& mi, QVariantMap& dataMap) const
{
  dataMap[QString::number(Qt::EditRole)] = data(mi, Qt::EditRole);

  QVariant checked = data(mi, Qt::CheckStateRole);
  if (checked.isValid())
    dataMap[QString::number(Qt::CheckStateRole)] = checked.toInt();

  QVariant iconAsString = data(mi, StandardItemModel::UserRoleIconAsString);
  if (iconAsString.isValid())
    dataMap[QString::number(StandardItemModel::UserRoleIconAsString)] = iconAsString;

  QVariant valueAsKey = data(mi, StandardItemModel::UserRoleKeyForValue);
  if (valueAsKey.isValid())
    dataMap[QString::number(StandardItemModel::UserRoleKeyForValue)] = valueAsKey;

  for (int role : StoredRoles)
  {
    QVariant value = data(mi, role);
    if (value.isValid())
    {
      dataMap[QString::number(role)] = value;
    }
  }
}

void StandardItemModel::SetLoading(bool loading)
{
  setProperty("Loading", loading);
}

bool StandardItemModel::GetLoading() const
{
  return property("Loading").toBool();
}

//// Lock / Unlock (save / load high level methods)
void StandardItemModel::Lock()
{
  SaveLoadMutex.lock();
}

void StandardItemModel::Unlock() 
{
  SaveLoadMutex.unlock();
}

void StandardItemModel::Reset()
{
  beginResetModel();
  endResetModel();
}

void StandardItemModel::SortRows(const QModelIndex& parent, int column)
{
  //// Avoid reenter
  QStringList sorting = property("Sorting").toStringList();
  QString parentAsString = GetKeyFromModelIndex(parent);
  if (sorting.contains(parentAsString))
    return;

  {
    //// Avoid reenter
    sorting << parentAsString;
    setProperty("Sorting", sorting);

    {
      //// Do sort
      setProperty("Loading", true);

#ifndef DISABLE_STANDARD_MODELS_SERIALIZING

      //// Save
      std::multimap<QVariant, QSharedPointer<rapidjson::Document>> rowsData;
      int rc = rowCount(parent);
      for (int r = 0; r < rc; ++r)
      {
        QModelIndex keyInRowModelIndex = index(r, column, parent);
        QVariant key = data(keyInRowModelIndex, Qt::EditRole);

        QSharedPointer<rapidjson::Document> rowData(new rapidjson::Document);
        rowData->SetArray();

        SaveRow(parent, r, *rowData, rowData->GetAllocator());
        rowsData.insert({ key, rowData });
      }

      //// Load
      {
        QSignalBlocker sb(this);
        int r = 0;
        for (auto storedRow = rowsData.begin(); storedRow != rowsData.end(); ++storedRow)
        {
          QSharedPointer<rapidjson::Document> restored = storedRow->second;
          LoadRow(parent, r, *restored);
          ++r;
        }
      }

#endif // !DISABLE_STANDARD_MODELS_SERIALIZING

      emit dataChanged(index(0, column, parent), index(rowCount() - 1, column, parent));

      setProperty("Loading", false);

    }

    //// Avoid reenter
    sorting.removeAll(parentAsString);
    setProperty("Sorting", sorting);
  }
}

//////////////////////////////////////////////////////////////////////
//// Reimplemented
QVariant StandardItemModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid())
  {
    switch (role)
    {
    case UserRoleColumnKey:
      return GetStaticColumnKey(index);

    case UserRoleClipboardDataRoleId:
      //// Intercapt clipboard
      return UserRoleClipboardDataRole;

    case UserRoleClipboardDataRole:
      {
        //// It depends...
        ColumnInformation column = GetColumnInformation(GetColumnKey(index));

        //// Provide icon path for clipboard
        if (column.Format.IconDisplay)
        {
          return data(index, StandardItemModel::UserRoleIconAsString);
        }
        if (!column.DataConnection.IsNull())
        {
          QVariant currentData = data(index, Qt::EditRole);
          for (int r = 0; r < column.DataConnection.SourceModelPtr->rowCount(); ++r)
          {
            QModelIndex sourceModelIndexKey   = column.DataConnection.SourceModelPtr->index(r, column.DataConnection.SourceIndexKey);
            QModelIndex sourceModelIndexName  = column.DataConnection.SourceModelPtr->index(r, column.DataConnection.SourceIndexName);
            if (sourceModelIndexKey.isValid()
             && sourceModelIndexName.isValid())
            {
              QVariant key = column.DataConnection.SourceModelPtr->data(sourceModelIndexKey, Qt::EditRole);
              if (key == currentData)
              {
                return column.DataConnection.SourceModelPtr->data(sourceModelIndexName).toString();
              }
            }
          }
        }

        //// Nothing special, return data from displayrole
        return data(index, Qt::DisplayRole);
      }
      break;
    default:
      break;
    }
  }
  return StandardItemModelBase::data(index, role);
}

bool StandardItemModel::CanAcceptClipboardData(const QModelIndex& index, const ColumnInformation& columnInformation, const QVariant& data)
{
  //// Reject invalid
  if (!index.isValid())
    return false;

  //// Reject for disabled
  auto f = flags(index);
  if (!f.testFlag(Qt::ItemIsEditable) || !f.testFlag(Qt::ItemIsEnabled))
    return false;

  //// Reject for non editable
  if (!columnInformation.Editable)
    return false;

  //// Reject paste to column containing unique value
  ModelValidation modelValidation(this);
  if (!modelValidation.IsNull()
    && !index.parent().isValid()
    && modelValidation.Uniqueness.Column == index.column())
    return false;

  //// Reject data than wont fit predefined values
  GenericPredefinedValues predefineValues = GetColumnPredefinedValues(index);
  if (!predefineValues.IsNull()
    && !predefineValues.Editable
    && !predefineValues.Values.contains(data.toString()))
    return false;

  //// Reject data that wont fit extern connection
  if (!columnInformation.DataConnection.IsNull()
    && !columnInformation.DataConnection.Edit)
  {
    QSet<QString> acceptableValues;
    acceptableValues << QString();

    if (columnInformation.DataConnection.SourceModelPtr)
    {
      for (int r = 0; r < columnInformation.DataConnection.SourceModelPtr->rowCount(); ++r)
      {
        QModelIndex sourceModelIndexName = columnInformation.DataConnection.SourceModelPtr->index(r, columnInformation.DataConnection.SourceIndexName);
        acceptableValues << columnInformation.DataConnection.SourceModelPtr->data(sourceModelIndexName).toString();
      }
    }

    if (!acceptableValues.contains(data.toString()))
      return false;
  }
  return true;
}

bool StandardItemModel::setData(const QModelIndex& idx, const QVariant& data, int role)
{
  if (idx.isValid())
  {
    switch (role)
    {
    case UserRoleClipboardDataRole:
      {
        //// Get column info
        ColumnInformation columnInformation       = GetColumnInformation(GetColumnKey(idx));
        GenericPredefinedValues predefinedValues  = GetColumnPredefinedValues(idx);

        //// Validate
        if (!CanAcceptClipboardData(idx, columnInformation, data))
          return false;

        //// It depends...
        if (!predefinedValues.IsNull())
        {
          if (predefinedValues.Values.contains(data.toString()))
          {
            setData(idx, data.toString(), Qt::EditRole);
            setData(idx, predefinedValues.Values.value(data.toString()), UserRoleKeyForValue);
          }
          else
          {
            if (predefinedValues.Editable)
            {
              setData(idx, data.toString(), Qt::EditRole);
              setData(idx, QString(), UserRoleKeyForValue);
            }
          }
        }
        else if (columnInformation.Format.IconDisplay)
        {
          return setData(idx, data, StandardItemModel::UserRoleIconAsString);
        }
        else if (!columnInformation.DataConnection.IsNull())
        {
          QVariant text = data.toString();
          for (int r = 0; r < columnInformation.DataConnection.SourceModelPtr->rowCount(); ++r)
          {
            QModelIndex sourceIndexKey  = columnInformation.DataConnection.SourceModelPtr->index(r, columnInformation.DataConnection.SourceIndexKey);
            QModelIndex sourceIndexName = columnInformation.DataConnection.SourceModelPtr->index(r, columnInformation.DataConnection.SourceIndexName);
            if (sourceIndexKey.isValid()
             && sourceIndexName.isValid())
            {
              QString name = columnInformation.DataConnection.SourceModelPtr->data(sourceIndexName).toString();
              QVariant key = columnInformation.DataConnection.SourceModelPtr->data(sourceIndexKey, Qt::EditRole);
              if (name == text)
              {
                if (setData(idx, key, Qt::EditRole))
                {
                  //// Set siblings data
                  auto modelInformation = GetModelInformation();
                  const QMap<QString, int>& targetIndexes = columnInformation.DataConnection.TargetColumnIndexes.empty() ? modelInformation.ColumnIndexes : columnInformation.DataConnection.TargetColumnIndexes;

                  std::map<QModelIndex, QModelIndex> sourceToTargetCopyMapping;
                  for (auto sk = columnInformation.DataConnection.SourceColumnKeys.begin(); sk != columnInformation.DataConnection.SourceColumnKeys.end(); ++sk)
                  {
                    if (targetIndexes.contains(sk.value()))
                    {
                      int sourceColumn    = sk.key();
                      int sourceRow       = sourceIndexKey.row();
                      QModelIndex source  = columnInformation.DataConnection.SourceModelPtr->index(sourceRow, sourceColumn);

                      int targetColumn    = targetIndexes.value(sk.value());
                      int targetRow       = idx.row();
                      QModelIndex target  = index(targetRow, targetColumn);

                      if (source.isValid() && target.isValid())
                      {
                        sourceToTargetCopyMapping[source] = target;
                      }
                    }
                  }

                  // Copy data
                  for (auto mapping = sourceToTargetCopyMapping.begin(); mapping != sourceToTargetCopyMapping.end(); ++mapping)
                  {
                    // Copy roles
                    std::set<Qt::ItemDataRole> roles;
                    roles.insert(Qt::EditRole);
                    roles.insert(Qt::DisplayRole);
                    roles.insert(Qt::DecorationRole);
                    roles.insert(Qt::UserRole);
                    roles.insert((Qt::ItemDataRole)StandardItemModel::UserRoleIconAsString);
                    roles.insert((Qt::ItemDataRole)StandardItemModel::UserRoleKeyForValue);

                    auto role = roles.begin();
                    while (role != roles.end())
                    {
                      QVariant sourceData = columnInformation.DataConnection.SourceModelPtr->data(mapping->first, *role);
                      setData(mapping->second, sourceData, *role);
                      ++role;
                    }
                  }
                  return true;
                }
              }
            }
          }
        }
        else
        {
          if (data.canConvert(QVariant::Double))
          {
            //// Reject data that wont fit validation
            GenericValidation validation = GetColumnValidation(idx);
            if (!validation.IsNull())
            {
              double d = data.toDouble();
              if (!qIsNaN(validation.InputMinimum)
                  && (d < validation.InputMinimum))
                return setData(idx, validation.InputMinimum, Qt::EditRole);

              if (!qIsNaN(validation.InputMaximum)
                  && (d > validation.InputMaximum))
                return setData(idx, validation.InputMaximum, Qt::EditRole);
            }
          }
        }

        //// Nothing special, paste data to edit role
        return setData(idx, data, Qt::EditRole);
      }
      break;
    case Qt::EditRole:
    case Qt::DisplayRole:
      /*
      Когда меняется ключ строки то обхекты, использующие этот ключ для связи с данными
      не могут получить информацию о том, как обновить связь. Другие сигналы высылают
      только новое значение, старое утрачивается. Чтобы этого не происходило высылается
      отдельный сигнал при обновлении строки, отмесченной как уникальная
      */
      {
        const QVariant unicColumnValue(this->property(KeyModelKeyColumnId));
        bool isUnicDefined;
        int unicColumn(unicColumnValue.toInt(&isUnicDefined));
        if (isUnicDefined && unicColumn == idx.column()) {
          const QVariant currentValue = this->data(idx, role);
          if (currentValue == data) {
            return true;
          }
          const bool updateResult(StandardItemModelBase::setData(idx, data, role));
          if (updateResult) {
            emit KeyDataChanged(idx, currentValue, data);
          }
          return updateResult;
        }
      }
      break;
    default:
      break;
    }
  }
  return StandardItemModelBase::setData(idx, data, role);
}

QVariant StandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal)
  {
    switch (role)
    {
    case UserRoleStringSizeHint:
      return GetColumnInformation(section).Scheme.TypicalValue;
    case UserRoleColumnKey:
      return GetStaticColumnKey(section);
    default:
      break;
    }
  }
  return StandardItemModelBase::headerData(section, orientation, role);
}

//// Notifications
void StandardItemModel::NotifyModelInformationChanged()
{
  emit ModelInformationChanged();
}

//////////////////////////////////////////////////////////////////////
//// Index to string / from string
QString StandardItemModel::GetKeyFromModelIndex(const QModelIndex& index) const
{
  if (!index.isValid())
    return "root";
  return std::move(GetKeyFromModelIndex(index.parent()) + "/"
                   + QString::number(index.row()) + ":" + QString::number(index.column()));
}

QModelIndex StandardItemModel::GetModelIndexFromKey(const QString& key, const QModelIndex& parent) const
{
  QStringList path = key.split("/");
  if (path.size() <= 1)
    return QModelIndex();

  QString pathRoot = path.takeFirst();
  QStringList pathRowColumn = pathRoot.split(":");
  if (pathRowColumn.size() == 2)
  {
    QModelIndex pathRootIndex = index(pathRowColumn[0].toInt(), pathRowColumn[1].toInt(), parent);
    return GetModelIndexFromKey(path.join("/"), pathRootIndex);
  }
  return QModelIndex();
}

void StandardItemModel::SetKeyColumnId(const int v) {
  this->setProperty(KeyModelKeyColumnId, v);
}

int StandardItemModel::GetKeyColumnId() const
{
  return this->property(KeyModelKeyColumnId).toInt();
}

void StandardItemModel::Clone(const QModelIndex& sourceIndex, const QModelIndex& targetIndex)
{
  Q_ASSERT(sourceIndex.model() == this);
  Q_ASSERT(targetIndex.model() == this);
  ModelDuplication duplication(this);
  for (int column : duplication.Duplicatable)
  {
    QModelIndex src = index(sourceIndex.row(), column);
    QModelIndex dst = index(targetIndex.row(), column);
    if (src.isValid() && dst.isValid())
    {
      StandardItemModel::Clone(this, src, this, dst);
    }
  }
}

void StandardItemModel::Clone(QAbstractItemModel* const source, const QModelIndex& sourceIndex, QAbstractItemModel* const target, const QModelIndex& targetIndex)
{
  Q_ASSERT(sourceIndex.model() == source);
  Q_ASSERT(targetIndex.model() == target);
  //// Clone the following roles (order is valuable)
  for (const Qt::ItemDataRole& r : {
    Qt::DisplayRole,
    Qt::EditRole,
    Qt::CheckStateRole,
    Qt::DecorationRole,
    Qt::UserRole,
    (Qt::ItemDataRole)StandardItemModel::UserRoleIconAsString,
    (Qt::ItemDataRole)StandardItemModel::UserRoleKeyForValue,
    })
  {
    const QVariant datum(source->data(sourceIndex, r));
    if (datum.isValid())
      target->setData(targetIndex, datum, r);
  }

  int rc = source->rowCount(sourceIndex);
  int cc = source->columnCount(sourceIndex);

  while (target->rowCount(targetIndex) < rc)
    target->insertRow(target->rowCount(targetIndex), targetIndex);
  while (target->rowCount(targetIndex) > rc)
    target->removeRow(target->rowCount(targetIndex) - 1, targetIndex);
  while (target->columnCount(targetIndex) < cc)
    target->insertColumn(target->columnCount(targetIndex), targetIndex);
  while (target->columnCount(targetIndex) > cc)
    target->removeColumn(target->columnCount(targetIndex) - 1, targetIndex);

  QSignalBlocker l(source);
  for (int r = 0; r < rc; ++r)
  {
    for (int c = 0; c < cc; ++c)
    {
      QModelIndex sourceChild = source->index(r, c, sourceIndex);
      QModelIndex targetChild = target->index(r, c, targetIndex);

      Q_ASSERT(sourceChild.isValid() && targetChild.isValid());
      if (sourceChild.isValid() && targetChild.isValid())
      {
        Clone(source, sourceChild, target, targetChild);
      }
    }
  }
}
