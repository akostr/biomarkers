#pragma once
#ifndef JOINT_PASSPORT_FORM_CONTROL_H
#define JOINT_PASSPORT_FORM_CONTROL_H

#include "itab_order_list.h"
#include <logic/structures/passport_form_structures.h>
#include <QPointer>

namespace Ui
{
  class JointPassportFormControl;
}

class PassportModelsManager;
class JointPassportFormControl : public iTabOrderWidget
{
  Q_OBJECT

public:
  explicit JointPassportFormControl(QWidget* parent = nullptr);
  virtual ~JointPassportFormControl();


  void FillField(const QStringList& fields);
  QString SelectedField() const;

  void FillCluster(const QStringList& clusters);
  QString SelectedCluster() const;

  void FillWells(const QStringList& wells);
  QString SelectedWell() const;

  QString SelectedDepthType();

  QDateTime SelectedDate() const;

  Passport::SampleGeoMainJoint Data() const;
  void SetData(const Passport::SampleGeoMainJoint &data);
  void SetModelsManager(QPointer<PassportModelsManager> manager);


protected:
  Ui::JointPassportFormControl* ui = nullptr;
  QPointer<PassportModelsManager> mModelsManager;

  void SetupUi();


  // iTabOrderList interface
public:
  QWidgetList tabOrderWidgets() override;
};
#endif
