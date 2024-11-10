#ifndef WEBDIALOGPLSCONCENTRATIONSETTINGS_H
#define WEBDIALOGPLSCONCENTRATIONSETTINGS_H

#include "ui/dialogs/templates/dialog.h"

class QRadioButton;
class QComboBox;

class WebDialogPLSConcentrationSettings : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  enum ConcentrationGatheringMethod
  {
    none,
    manual,
    autoFromLayers,
    autoRandomly
  };

public:
  explicit WebDialogPLSConcentrationSettings(QWidget* parent);
  ~WebDialogPLSConcentrationSettings() override = default;

  ConcentrationGatheringMethod getConcentrationGatheringMethod();
  bool isManual();
  void FillLayers(const QStringList& layerNames);
  QString SelectedLayer() const;

private:
  void SetupUi();

  QPointer<QRadioButton> ManualRadio;
  QPointer<QRadioButton> AutoFromLayers;
  QPointer<QRadioButton> AutoRandomly;
  QPointer<QComboBox> Layers;
};

#endif // WEBDIALOGPLSCONCENTRATIONSETTINGS_H
