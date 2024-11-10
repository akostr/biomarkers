#pragma once

#include "view.h"

#include "../logic/tree_model_dynamic_job_functions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QRadioButton>
#include <QStackedWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

////////////////////////////////////////////////////
//// LoginEdit
namespace Views
{
class ViewPageLoginEdit : public View
{
  Q_OBJECT

public:
  ViewPageLoginEdit(QWidget* parent = 0);
  ~ViewPageLoginEdit();

  void SetupModels();
  void SetupUi();

  //// Handle user context
  virtual void ApplyContextUser(const QString& dataId, const QVariant& data) override;

private:
  //// Models
  QPointer<TreeModelDynamicJobFunctions> JobFunctions;

  //// Ui
  QPointer<QVBoxLayout>       Layout;
  QPointer<QHBoxLayout>       InnerLayout;

  QPointer<QFrame>            Content;
  QPointer<QVBoxLayout>       ContentLayout;

  QPointer<QStackedWidget>    ContentStack;

  QPointer<QWidget>           PersonalData;
  QPointer<QVBoxLayout>       PersonalDataLayout;
  QPointer<QHBoxLayout>       PersonalDataForm;
  QPointer<QVBoxLayout>       PersonalDataFormLeft;
  QPointer<QVBoxLayout>       PersonalDataFormRight;
  QPointer<QLineEdit>         PersonalDataFirstName;
  QPointer<QLineEdit>         PersonalDataLastName;
  QPointer<QLineEdit>         PersonalDataPhone;
  QPointer<QComboBox>         PersonalDataFunction;
  QPointer<QLabel>            PersonalDataEmail;
  QPointer<QDialogButtonBox>  PersonalDataAccept;

  QPointer<QWidget>           NotificationSettings;
  QPointer<QVBoxLayout>       NotificationSettingsLayout;

  QPointer<QVBoxLayout>       MenuAreaLayout;
  QPointer<QFrame>            Menu;
  QPointer<QVBoxLayout>       MenuLayout;

  QPointer<QRadioButton>      MenuButtonPersonalData;
  QPointer<QRadioButton>      MenuButtonNotificationSettings;
};
}//namespace Views
