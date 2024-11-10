#ifndef WEBDIALOGANALYSISMODELSETTINGS_H
#define WEBDIALOGANALYSISMODELSETTINGS_H

#include "ui/dialogs/templates/dialog.h"

class QCheckBox;
class WebDialogAnalysisModelSettings : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  enum Option
  {
    NoOptions = 0x0,
    FilterPKEnabled = 0x1,
    Autoscale = 0x2,
    Normalization = 0x4
  };
  Q_DECLARE_FLAGS(Options, Option);


public:
  WebDialogAnalysisModelSettings(QWidget *parent);
  void Accept() override;

  void AddEnabledOptions(const std::map<Option, bool>& opt);
  void AddDisabledOptions(const std::map<Option, bool>& opt);

  Options GetOptions();

signals:
  void OptionsAccepted(Options opt);

private:
  void SetupUi();

private:
  QPointer<QCheckBox> FilterCheckBox;
  QPointer<QCheckBox> AutoScaleStatus;
  QPointer<QCheckBox> NormStatus;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WebDialogAnalysisModelSettings::Options)

#endif // WEBDIALOGANALYSISMODELSETTINGS_H
