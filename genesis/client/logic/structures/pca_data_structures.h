#pragma once
#include <QString>
#include <QVector>
#include <QObject>

#include <ui/plots/gp_items/gpshape_item.h>

enum ParameterType
{
  Field,
  Well,
  Layer,
  Date,
  LastType
};

struct Component
{
  double ExplPCsVariance;
  QVector<double> P;
  QVector<double> T;
};

struct T_Title
{
  QString date;
  QString field;
  QString filetitle;
  QString filename;
  QString layer;
  QString well;
  QString depth;
  QString wellCluster;
  int fileId;
  int sampleId;
  bool excluded;
  QString getParamValue(ParameterType param) const
  {
    switch(param)
    {
    case Date: return date;
    case Field: return field;
    case Layer: return layer;
    case Well: return well;
    default: return "";
    }
  }
};

static QString ParameterToString(ParameterType param)
{
  switch(param)
  {
  case Field: return QObject::tr("Field", "PCA_DataTypes");
  case Well: return QObject::tr("Well", "PCA_DataTypes");
  case Layer: return QObject::tr("Layer", "PCA_DataTypes");
  case Date: return QObject::tr("Date", "PCA_DataTypes");
  default: return "";
  }
};

struct CustomItemStyle
{
  GPShape::ShapeType shape;
  GPShapeItem::DefaultColor color;
  QSizeF size;
};

struct ItemStyle
{
  GPShape::ShapeType shape;
  GPShapeItem::DefaultColor color;
};
