#include "sample_info_plate.h"

SampleInfoPlate::SampleInfoPlate(GraphicsPlot *parentPlot)
  : ParametersInfoPlate(parentPlot)
{
  AppendParameter(tr("Field"), "");
  AppendParameter(tr("Well"), "");
  AppendParameter(tr("Layer"), "");
  AppendParameter(tr("Date"), "");
}

void SampleInfoPlate::SetParameter(ParameterType param, const QString &value)
{
  switch(param)
  {
  case Field:
    SetParameterValue(tr("Field"), value);
    break;
  case Well:
    SetParameterValue(tr("Well"), value);
    break;
  case Layer:
    SetParameterValue(tr("Layer"), value);
    break;
  case Date:
    SetParameterValue(tr("Date"), value);
    break;
  default: return;
  }
}

QString SampleInfoPlate::GetParameter(ParameterType param) const
{
  switch(param)
  {
  case Field:
    return GetParameterValue(tr("Field"));
  case Well:
    return GetParameterValue(tr("Well"));
  case Layer:
    return GetParameterValue(tr("Layer"));
  case Date:
    return GetParameterValue(tr("Date"));
  default: return "";
  }
}

void SampleInfoPlate::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
  if(selectionStateChanged) *selectionStateChanged = false;

  if((additive || !selected()) && selectionStateChanged)
    *selectionStateChanged = true;
  if(additive)
    setSelected(!selected());
  else
    setSelected(true);
}

void SampleInfoPlate::deselectEvent(bool *selectionStateChanged)
{
  if(selectionStateChanged) *selectionStateChanged = false;

  if(selected())
  {
    if(selectionStateChanged) *selectionStateChanged = true;

    setSelected(false);
  }
}
