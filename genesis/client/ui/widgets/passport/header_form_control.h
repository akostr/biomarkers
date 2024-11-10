#pragma once
#ifndef PASSPORT_HEADER_FORM_CONTROL_H
#define PASSPORT_HEADER_FORM_CONTROL_H

#include "itab_order_list.h"
#include <logic/structures/passport_form_structures.h>
#include <QPointer>

namespace Ui
{
  class PassportHeaderFormControl;
}

class PassportModelsManager;
class PassportHeaderFormControl : public iTabOrderWidget
{
  Q_OBJECT

public:
  explicit PassportHeaderFormControl(QWidget* parent);
  virtual ~PassportHeaderFormControl();

  void SetComment(const QString& comment);
  QString Comment() const;

  void FillSamples(const QStringList& samples);
  QString SelectedSample() const;

  void FillFluids(const QStringList& fluids);
  QString SelectedFluid() const;

  void SetData(const Passport::SampleHeaderData& data);
  Passport::SampleHeaderData Data();
  void SetModelsManager(QPointer<PassportModelsManager> manager);

signals:
  void FluidChanged(Passport::FluidType fluidType);
  void SampleTypeChanged(Passport::SampleType sampleType);

private:
  Ui::PassportHeaderFormControl* ui = nullptr;
  QPointer<PassportModelsManager> mModelsManager;

  void SetupUi();
  void ConnectSignals();


  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;
};
#endif
