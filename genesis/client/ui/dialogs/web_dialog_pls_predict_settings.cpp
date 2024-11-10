#include "web_dialog_pls_predict_settings.h"
#include "ui_web_dialog_pls_predict_settings.h"
#include "genesis_style/style.h"

using namespace Structures;

namespace Dialogs
{
    WebDialogPlsPredictSettings::WebDialogPlsPredictSettings(QWidget* parent)
      : WebDialog(parent, nullptr, QSize(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
    {
      SetupUi();
      connect(DialogWidget->CloseButton, &QPushButton::clicked, this, &WebDialog::Reject);
      connect(DialogWidget->PlsModelComboBox, &QComboBox::editTextChanged, this, &WebDialogPlsPredictSettings::CheckLayer);
    }

    WebDialogPlsPredictSettings::~WebDialogPlsPredictSettings()
    {
      delete DialogWidget;
    }

    void WebDialogPlsPredictSettings::SetItems(const Structures::ParentAnalysisIds& parents)
    {
      Analysis = parents;
      DialogWidget->PlsModelComboBox->clear();
      QStringList names;
      std::transform(parents.begin(), parents.end(), std::back_inserter(names),
        [](const ParentAnalysisId& item) { return QString::fromStdString(item.Title); });
      DialogWidget->PlsModelComboBox->addItems(names);
      CheckLayer(DialogWidget->PlsModelComboBox->currentText());
    }

    QString WebDialogPlsPredictSettings::GetSelected() const
    {
      return DialogWidget->PlsModelComboBox->currentText();
    }

    bool WebDialogPlsPredictSettings::IsManual() const
    {
      return DialogWidget->ManualRadioButton->isChecked();
    }

    bool WebDialogPlsPredictSettings::IsEmpty() const
    {
      return DialogWidget->EmptyRadioButton->isChecked();
    }

    bool WebDialogPlsPredictSettings::IsAuto() const
    {
      return DialogWidget->AutoRadioButton->isChecked();
    }

    void WebDialogPlsPredictSettings::SetupUi()
    {
      const auto body = new QWidget();
      body->setStyleSheet(Style::Genesis::GetUiStyle());
      DialogWidget = new Ui::WebDialogPlsPredictSettings();
      DialogWidget->setupUi(body);
      DialogWidget->label->setStyleSheet(Style::Genesis::Fonts::ModalHead());
      DialogWidget->label_2->setStyleSheet(Style::Genesis::Fonts::RegularBold());
      DialogWidget->label_3->setStyleSheet(Style::Genesis::Fonts::RegularBold());
      DialogWidget->ManualRadioButton->toggle();
      DialogWidget->AutoRadioButton->setDisabled(true);
      ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Setup PLS Predict"));
      if (const auto layout = Content->layout())
      {
        layout->addWidget(body);
      }
    }

    void WebDialogPlsPredictSettings::CheckLayer(const QString& currenModelName)
    {
      const auto it = std::find_if(Analysis.begin(), Analysis.end(),
        [&](const ParentAnalysisId& item)
        {
          return item.Title == currenModelName.toStdString();
        });
      DialogWidget->AutoRadioButton->setEnabled(it != Analysis.end() && !it->LayerName.empty());
    }
}
