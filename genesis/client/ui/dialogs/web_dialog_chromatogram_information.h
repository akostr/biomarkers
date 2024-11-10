#pragma once

#include "web_dialog.h"

#include "../../logic/tree_model_dynamic_fields.h"
#include "../../logic/tree_model_dynamic_field_wells.h"
#include "../../logic/tree_model_dynamic_layers.h"
#include "../../logic/tree_model_dynamic_file_infos.h"

#include <QLabel>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLayout>

/////////////////////////////////////////////////////
//// Web Dialog / Chromatogram information
class WebDialogChromatogramInformation : public WebDialog
{
  Q_OBJECT

public:
  WebDialogChromatogramInformation(QWidget* parent, int fileId);
  ~WebDialogChromatogramInformation();

  virtual void Accept() override;
  virtual void Reject() override;

  bool DependenciesSafisfied();
  bool DependenciesSafisfiedField();
  bool DependenciesSafisfiedWell();
  bool DependenciesSafisfiedLayer();

  int GetDependencyField();
  int GetDependencyWell();
  int GetDependencyLayer();
  QString GetTitle();

  void CreateDependencies();
  void CreateDependencyField();
  void CreateDependencyWell();
  void CreateDependencyLayer();

  void SendRequest();

private:
  void SetupModels();
  void SetupUi();

private slots:
  void UpdateButtons();

private:
  //// Models
  int                                   FileId;
  QPointer<TreeModelDynamicFileInfos>   FileInfo;

  QPointer<TreeModelDynamicFields>      Fields;
  QPointer<TreeModelDynamicFieldWells>  FieldWells;
  QPointer<TreeModelDynamicLayers>      Layers;

  int CreatedFieldId;
  int CreatedWellId;
  int CreatedLayerId;

  //// Ui
  QPointer<QWidget>         Body;
  QPointer<QVBoxLayout>     BodyLayout;

  QPointer<QComboBox>       ComboField;
  QPointer<QComboBox>       ComboWell;
  QPointer<QComboBox>       ComboLayer;
  QPointer<QComboBox>       ComboWellCluster;
  QPointer<QDateTimeEdit>   EditDateTime;
  QPointer<QLineEdit>       EditTitle;
};
