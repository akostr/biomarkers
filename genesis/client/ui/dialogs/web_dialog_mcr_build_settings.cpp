#include "web_dialog_mcr_build_settings.h"

#include "ui_web_dialog_mcr_build_settings.h"

#include "genesis_style/style.h"
#include "ui/widgets/mcr_component_settings.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "logic/known_json_tag_names.h"

#include <QPushButton>
#include <QJsonDocument>

namespace
{
  const static int MinimumComponentNumber = 2;
}

using namespace Widgets;
using namespace Structures;

namespace
{
  const static std::string Etalon = "etalon";
  const static std::string MGK = "MGK";
}

namespace Dialogs
{
  WebDialogMCRBuildSettings::WebDialogMCRBuildSettings(QWidget* parent)
    : Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  {
    SetupUi();
    ConnectActions();
    InitDefaultSettings();
  }

  WebDialogMCRBuildSettings::~WebDialogMCRBuildSettings()
  {
    delete WidgetUi;
  }

  void WebDialogMCRBuildSettings::SetSampleIds(const IntVector& samples)
  {
    SampleIds = samples;
  }

  IntVector WebDialogMCRBuildSettings::GetSampleIds() const
  {
    return SampleIds;
  }

  void WebDialogMCRBuildSettings::SetMarkersIds(const Structures::IntVector& markers)
  {
    MarkersIds = markers;
  }

  IntVector WebDialogMCRBuildSettings::GetMarkerIds() const
  {
    return MarkersIds;
  }

  void WebDialogMCRBuildSettings::SetTableId(const int tableId)
  {
    TableId = tableId;
  }

  int WebDialogMCRBuildSettings::GetTableId() const
  {
      return TableId;
  }

  void WebDialogMCRBuildSettings::SetMaximumComponentCount(int max)
  {
    WidgetUi->ComponentCountSpinBox->setMaximum(max);
  }

  void WebDialogMCRBuildSettings::SetSampleNames(const QStringList& names)
  {
    SampleNames = names;
    SetMaximumComponentCount(names.size());
  }

  void WebDialogMCRBuildSettings::SetParameterType(const QString& param)
  {
    ParameterType = param;
  }

  QString WebDialogMCRBuildSettings::GetParameterType() const
  {
      return ParameterType;
  }

  ReferenceSettingsList WebDialogMCRBuildSettings::GetReferenceSettings() const
  {
    ReferenceSettingsList refList;
    const auto components = findChildren<MCRComponentSettings*>();
    std::transform(components.begin(), components.end(), std::back_inserter(refList),
      [&](const MCRComponentSettings* settings) -> ReferenceSettings
      {
        return
        {
          WidgetUi->SamplesRadioButton->isChecked()
            ? static_cast<size_t>(SampleIds[SampleNames.indexOf(settings->GetSelectedSample())])
            : -1,
          settings->IsNonNegativeConcentration(),
          settings->IsNonNegativeSpectral(),
          settings->IsFixed(),
        };
      });
    return refList;
  }

  void WebDialogMCRBuildSettings::SetReferenceSettings(const ReferenceSettingsList& settings)
  {
    ComponentValueChanged(0);

    for (const auto& item : settings)
    {
      const auto comp = AddComponent(SampleNames);
      comp->SetFixed(item.Fix ? Qt::Checked : Qt::Unchecked);
      comp->SetNonNegativeConcentration(item.NonNegativeConcentration ? Qt::Checked : Qt::Unchecked);
      comp->SetNonNegativeSpectral(item.NonNegativeSpectrum ? Qt::Checked : Qt::Unchecked);
      const auto it = std::find_if(SampleIds.begin(), SampleIds.end(),
        [&](const int& sampleName) { return item.ReferenceId == sampleName; });
      comp->SetSelectedSample(it == SampleIds.end() ? tr("Component not found") : SampleNames[std::distance(SampleIds.begin(), it)]);
    }
  }

  size_t WebDialogMCRBuildSettings::GetComponentCurrentValue() const
  {
    return WidgetUi->ComponentCountSpinBox->value();
  }

  void WebDialogMCRBuildSettings::SetComponetCurrentValue(size_t count)
  {
    WidgetUi->ComponentCountSpinBox->setValue(count);
  }

  bool WebDialogMCRBuildSettings::IsClosedSystem() const
  {
    return WidgetUi->ClosedSystemCheckBox->isChecked();
  }

  void WebDialogMCRBuildSettings::SetClosedSystem(bool isClosed) const
  {
    WidgetUi->ClosedSystemCheckBox->setChecked(isClosed);
  }

  bool WebDialogMCRBuildSettings::IsNormData() const
  {
    return WidgetUi->NormDataCheckBox->isChecked();
  }

  void WebDialogMCRBuildSettings::SetNormData(bool isNorm) const
  {
    WidgetUi->NormDataCheckBox->setChecked(isNorm);
  }

  size_t WebDialogMCRBuildSettings::GetMaxIterations() const
  {
    return WidgetUi->IterationCountSpinBox->value();
  }

  void WebDialogMCRBuildSettings::SetMaxIterations(size_t maxIters) const
  {
    WidgetUi->IterationCountSpinBox->setValue(maxIters);
  }

  double WebDialogMCRBuildSettings::GetEvaluatedValue() const
  {
    return ErrorChange[WidgetUi->ConvergenceComboBox->currentIndex()];
  }

  void WebDialogMCRBuildSettings::SetEvaluatedValue(double tolErrChange)
  {
    const auto it = std::find(ErrorChange.begin(), ErrorChange.end(), tolErrChange);
    if (it == ErrorChange.end())
      return;
    WidgetUi->ConvergenceComboBox->setCurrentIndex(std::distance(ErrorChange.begin(), it));
  }

  std::string WebDialogMCRBuildSettings::GetMethod() const
  {
    if (WidgetUi->SamplesRadioButton->isChecked())
      return Etalon;
    return MGK;
  }

  void WebDialogMCRBuildSettings::SetMethod(const std::string& method)
  {
    WidgetUi->SamplesRadioButton->setChecked(method == Etalon);
    WidgetUi->EvaluationRadioButton->setChecked(method == MGK);
  }

  void WebDialogMCRBuildSettings::SetupUi()
  {
    Dialogs::Templates::Dialog::Settings s;
    s.dialogHeader = tr("MCR model parameters");
    s.buttonsNames = { {QDialogButtonBox::Ok, tr("Build MCR")},
                     {QDialogButtonBox::Cancel, tr("Cancel")} };
    applySettings(s);

    const auto body = new QWidget();
    WidgetUi = new Ui::WebDialogMCRBuildSettings();
    WidgetUi->setupUi(body);
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (const auto layout = Content->layout())
    {
      layout->addWidget(body);
    }

    ErrorChange =
    {
        0.1,
        0.01,
        0.001,
        0.0001,
        0.00001,
        0.000001,
        0.0000001,
        0.00000001,
        0.000000001,
        0.0000000001,
    };
    WidgetUi->ConvergenceComboBox->addItems(
      {
        "0,1 (1e-10)",
        "0,01 (1e-100)",
        "0,001 (1e-1000)",
        "0,0001 (1e-10000)",
        "0,00001 (1e-100000)",
        "0,000001 (1e-1000000)",
        "0,0000001 (1e-10000000)",
        "0,00000001 (1e-100000000)",
        "0,000000001 (1e-1000000000)",
        "0,0000000001 (1e-10000000000)",
      });

    WidgetUi->label_blue->setPixmap(QPixmap("://resource/icons/blue_square.png"));
    WidgetUi->label_red->setPixmap(QPixmap("://resource/icons/red_square.png"));
    WidgetUi->label_EFAEstimation->setStyleSheet(Style::Genesis::GetH2());
    WidgetUi->AnalysisBaseLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    WidgetUi->CommonSettingsLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    WidgetUi->IterationCountLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
    WidgetUi->CoonvergenceLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
    WidgetUi->label_additionally->setStyleSheet(Style::Genesis::Fonts::RegularBold());
    WidgetUi->pushButton_calculateEFA->setStyleSheet(Style::ApplySASS("QPushButton { background-color:@uiInputElementColorBgDisabled; }"));
  }

  void WebDialogMCRBuildSettings::ConnectActions()
  {
    connect(WidgetUi->EvaluationRadioButton, &QRadioButton::toggled, this, &WebDialogMCRBuildSettings::EvaluatedRadioButtonToggled);
    connect(WidgetUi->ComponentCountSpinBox, &QSpinBox::valueChanged, this, &WebDialogMCRBuildSettings::ComponentValueChanged);
    connect(WidgetUi->ComponentCountSpinBox, &QSpinBox::valueChanged, this, &WebDialogMCRBuildSettings::CheckBuildEnabled);
    connect(WidgetUi->pushButton_calculateEFA, &QPushButton::clicked, this, &WebDialogMCRBuildSettings::CalculateEFA);
  }

  void WebDialogMCRBuildSettings::InitDefaultSettings()
  {
    WidgetUi->ComponentCountSpinBox->setMinimum(0);
    WidgetUi->EvaluationRadioButton->toggle();
    emit WidgetUi->tabWidget->currentChanged(0);
  }

  void WebDialogMCRBuildSettings::EvaluatedRadioButtonToggled(bool checked)
  {
    const auto components = findChildren<MCRComponentSettings*>();
    for (const auto& component : components)
    {
      component->SetSampleListVisible(!checked);
      component->SetStarVisible(!checked);
    }
  }

  MCRComponentSettings* WebDialogMCRBuildSettings::AddComponent(const QStringList& list)
  {
    const auto component = new MCRComponentSettings(this);
    const auto count = findChildren<MCRComponentSettings*>().size();
    component->SetSampleList(list);
    component->SetLabelText(tr("Component %1").arg(count));
    WidgetUi->ScrollAreaLayout->layout()->addWidget(component);
    component->SetSampleListVisible(WidgetUi->SamplesRadioButton->isChecked());
    component->SetStarVisible(WidgetUi->SamplesRadioButton->isChecked());
    return component;
  }

  void WebDialogMCRBuildSettings::ComponentValueChanged(int currentValue)
  {
    auto components = findChildren<MCRComponentSettings*>();
    auto diff = currentValue - static_cast<int>(components.size());
    if (diff > 0)
    {
      while (diff-- > 0)
        AddComponent(SampleNames);
    }
    else if (diff < 0)
    {
      const auto layout = WidgetUi->ComponentSettings->layout();
      while (diff++ < 0)
      {
        auto last = components.takeLast();
        layout->removeWidget(last);
        delete last;
      }
    }
    UpdateGeometry();
  }

  void WebDialogMCRBuildSettings::CalculateEFA()
  {
    QJsonObject jroot;
    jroot[JsonTagNames::Key] = ParameterType;
    jroot[JsonTagNames::Normdata] = WidgetUi->NormDataCheckBox->isChecked();

    QJsonArray sampleIds;
    std::transform(SampleIds.begin(), SampleIds.end(), std::back_inserter(sampleIds), [](int value) { return value; });
    jroot[JsonTagNames::SamplesIds] = sampleIds;

    QJsonArray markersIds;
    std::transform(MarkersIds.begin(), MarkersIds.end(), std::back_inserter(markersIds), [](int value) { return value; });
    jroot[JsonTagNames::MarkersIds] = markersIds;
    jroot[JsonTagNames::TableId] = TableId;

    API::REST::CalculateEFA(jroot, [&](QNetworkReply*, QJsonDocument doc)
    {
      auto responseJson = doc.object();
      if (responseJson.value(JsonTagNames::Error).toBool())
      {
        Notification::NotifyError(responseJson.value(JsonTagNames::Msg).toString(), tr("EFA error."));
        return;
      }

      //// parse
      auto jobj = doc.object();
      if (jobj.contains(JsonTagNames::Error)
        && jobj[JsonTagNames::Error].isBool()
        && jobj[JsonTagNames::Error].toBool())
      {
        Notification::NotifyError(tr("Response json error"));
        return;
      }

      WidgetUi->widget_EFAPlot->clearCurves();

      if (jobj.contains(JsonTagNames::Result) && jobj[JsonTagNames::Result].isObject())
      {
        auto result = jobj[JsonTagNames::Result].toObject();

        if (result.contains("forward") && result["forward"].isObject())
        {
          auto forward = result["forward"].toObject();

          for (const auto& it : forward)
          {
            QVector<double> xForwardKeys;
            QVector<double> yForwardKeys;
            int x{};

            if (it.isArray())
            {
              auto arr = it.toArray();
              for (size_t i = 0; i < arr.size(); i++)
              {
                auto d = arr[i].toDouble();
                xForwardKeys << ++x;
                yForwardKeys << d;
              }
            }

            WidgetUi->widget_EFAPlot->AppendCurve(xForwardKeys, yForwardKeys, Qt::red);
          }
        }

        if (result.contains("backward") && result["backward"].isObject())
        {
          for (const auto& it : result["backward"].toObject())
          {
            QVector<double> xForwardKeys;
            QVector<double> yForwardKeys;
            int x{};

            if (it.isArray())
            {
              auto arr = it.toArray();
              for (size_t i = 0; i < arr.size(); i++)
              {
                auto d = arr[i].toDouble();
                xForwardKeys << ++x;
                yForwardKeys << d;
              }
            }

            WidgetUi->widget_EFAPlot->AppendCurve(xForwardKeys, yForwardKeys, Qt::blue);
          }
        }
      }

      WidgetUi->widget_EFAPlot->rescaleAxes();
      WidgetUi->widget_EFAPlot->replot();
    },
    [](QNetworkReply*, QNetworkReply::NetworkError err)
    {
      Notification::NotifyError(tr("Network error"), err);
    });
  }

  void WebDialogMCRBuildSettings::CheckBuildEnabled()
  {
    if(const auto button = ButtonBox->button(QDialogButtonBox::Ok))
      button->setEnabled(WidgetUi->ComponentCountSpinBox->value() >= MinimumComponentNumber);
  }
}
