#pragma once
#ifndef GPEXSAMPLEINFOPLATE_H
#define GPEXSAMPLEINFOPLATE_H

#include "logic/structures/pca_data_structures.h"
#include "parameters_info_plate.h"

//TODO make single item with shape and plate.
class SampleInfoPlate : public ParametersInfoPlate
{
  Q_OBJECT
public:
  SampleInfoPlate(GraphicsPlot *parentPlot);

  void SetParameter(ParameterType param, const QString& value);
  QString GetParameter(ParameterType param) const;
  void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) override;
  void deselectEvent(bool *selectionStateChanged) override;
};
#endif // GPEXSAMPLEINFOPLATE_H
