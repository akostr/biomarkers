#pragma once
#ifndef PASSPORT_FORM_CONTROL_H
#define PASSPORT_FORM_CONTROL_H

#include "itab_order_list.h"
#include <logic/structures/passport_form_structures.h>
#include <QPointer>

namespace Ui
{
  class PassportFormControl;
}

class PassportModelsManager;
class PassportFormControl : public iTabOrderWidget
{
  Q_OBJECT

public:
  explicit PassportFormControl(QWidget* parent);
  virtual ~PassportFormControl();

  void FillField(const QStringList& fields);
  QString SelectedField() const;

  void FillCluster(const QStringList& clusters);
  QString SelectedCluster() const;

  void FillWells(const QStringList& wells);
  QString SelectedWell() const;

  void FillLayers(const QStringList& layers);
  QString SelectedLayer() const;

  QString SelectedDepthType();

  QDateTime SelectedDate() const;

  Passport::SampleGeoMainNonMixed Data();
  void SetData(const Passport::SampleGeoMainNonMixed& data);

  void Clear();
  void SetModelsManager(QPointer<PassportModelsManager> manager);

private:
    Ui::PassportFormControl* ui = nullptr;
    QPointer<PassportModelsManager> mModelsManager;

    void SetupUi();

    // iTabOrderList interface
public:
    QWidgetList tabOrderWidgets() override;
};
#endif
