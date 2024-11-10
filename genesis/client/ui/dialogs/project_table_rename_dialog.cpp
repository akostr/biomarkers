#include "project_table_rename_dialog.h"
#include "ui_project_table_rename_dialog.h"
#include <genesis_style/style.h>
#include <logic/notification.h>
#include <api/api_rest.h>
#include <ui/genesis_window.h>
#include <QPainter>

namespace Dialogs
{
enum NameState
{
  state_none = 0,
  state_unchecked,
  state_valid,
  state_invalid
};

ProjectTableRenameDialog::ProjectTableRenameDialog(QWidget *parent, int projectId, QString projectTitle) :
  WebDialog(parent, nullptr, QSize(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::ProjectTableRenameDialog),
  mPMValid("://resource/styles/standard/qmenu/checkbox_context_on@2x.png"),
  mPMInvalid("://resource/styles/standard/icons/input_error_warning@2x.png"),
  mPMUnchecked("://resource/styles/standard/qmenu/checkbox_context_off@2x.png"),
  mContent(new QWidget(Content))
{
  ui->setupUi(mContent);
  setupUi();
  auto s = Settings();
  s.defaultTitle = projectTitle;
  s.projectId = projectId;
  applySettings(s);
  Content->layout()->addWidget(mContent);
}

ProjectTableRenameDialog::ProjectTableRenameDialog(QWidget *parent, const Settings &settings)
 : WebDialog(parent, nullptr, QSize(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::ProjectTableRenameDialog),
  mPMValid("://resource/styles/standard/qmenu/checkbox_context_on@2x.png"),
  mPMInvalid("://resource/styles/standard/icons/input_error_warning@2x.png"),
  mPMUnchecked("://resource/styles/standard/qmenu/checkbox_context_off@2x.png"),
  mContent(new QWidget(this))
{
  ui->setupUi(mContent);
  setupUi();
  applySettings(settings);
  Content->layout()->addWidget(mContent);
}

ProjectTableRenameDialog::~ProjectTableRenameDialog()
{
  delete ui;
}

void ProjectTableRenameDialog::applySettings(const Settings &newSettings)
{
  mSettings = newSettings;
  auto& s = mSettings;

  auto titleSettings = ui->titleWidget->settings();
  titleSettings.defaultText = s.defaultTitle;
  titleSettings.textHeader = s.titleHeader;
  titleSettings.textMaxLen = s.titleMaxLen;
  titleSettings.textPlaceholder = s.titlePlaceholder;
  titleSettings.textRequired = s.titleRequired;
  titleSettings.controlSymbols = s.controlSymbols;
  titleSettings.textTooltipsSet = {s.titleTooltipsSet.empty,
                                   s.titleTooltipsSet.forbidden,
                                   s.titleTooltipsSet.notChecked,
                                   s.titleTooltipsSet.valid,
                                   s.titleTooltipsSet.invalidSymbol};
  titleSettings.forbiddenStrings = s.forbiddenTitles;
  ui->titleWidget->applySettings(titleSettings);
  ButtonBox->button(QDialogButtonBox::Ok)->setText(s.okBtnText);
  ButtonBox->button(QDialogButtonBox::Cancel)->setText(s.cancelBtnText);

  ui->captionLabel->setText(s.header);
  ui->statusLabel->setText(s.statusHeader);
}

ProjectTableRenameDialog::Settings ProjectTableRenameDialog::settings()
{
  return mSettings;
}

QString ProjectTableRenameDialog::getTitle()
{
  return ui->titleWidget->text();
}

QString ProjectTableRenameDialog::getStatusCode()
{
  return ui->statusCombo->currentData().toString();
}

QString ProjectTableRenameDialog::getStatus()
{
  return ui->statusCombo->currentText();
}

ProjectTableRenameDialog::Settings ProjectTableRenameDialog::defaultSettings()
{
  return Settings();
}

void ProjectTableRenameDialog::setupUi()
{
  ui->captionLabel->setStyleSheet(Style::Genesis::GetH1_5());
  ui->statusLabel->setStyleSheet(Style::Genesis::GetH4());
  connect(ui->closeButton, &QPushButton::clicked, this, &WebDialog::Reject);
  connect(ui->titleWidget, &DialogLineEditEx::validityChanged, this, &ProjectTableRenameDialog::setDialogDataValidity);
}

void ProjectTableRenameDialog::setDialogDataValidity(bool isValid)
{
  auto btn = ButtonBox->button(QDialogButtonBox::Ok);
  if(isValid && !btn->isEnabled())
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
  else if(!isValid && btn->isEnabled())
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
}

void ProjectTableRenameDialog::loadModels()
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  //some api request
  auto callback = [overlayId, this](QNetworkReply*, QJsonDocument doc)
  {
    auto root = doc.object();
    if(root["error"].toBool() == true)
    {
      Notification::NotifyError(root["msg"].toString(), tr("Server error"));
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Reject();
      return;
    }
    auto jforbiddenArray = root["project_titles"].toArray();
    auto jvariantsArray = root["status_variants"].toArray();

    mSettings.forbiddenTitles.clear();
    for(const auto& v : jforbiddenArray)
    {
      if(v == mSettings.defaultTitle)
        continue;
      mSettings.forbiddenTitles << v.toString();
    }
    applySettings(mSettings);

    for(const auto& v : jvariantsArray)
    {
      auto a = v.toArray();
      QIcon icon = createCircleIcon({10, 10}, QColor(a[2].toString()));
      ui->statusCombo->addItem(icon, a[1].toString(), a[0].toString());
    }

    GenesisWindow::Get()->RemoveOverlay(overlayId);
  };
  auto errorCallback = [overlayId, this](QNetworkReply*, QNetworkReply::NetworkError e)
  {
    Notification::NotifyError(tr("Can't load data for proper renaming. Check your connection."), tr("Network error"), e);
    GenesisWindow::Get()->RemoveOverlay(overlayId);
    Reject();
  };

  API::REST::GetOccupiedProjectsNames(callback, errorCallback);
}

QIcon ProjectTableRenameDialog::createCircleIcon(QSize size, QColor color)
{
  QPixmap pmap(size);
  pmap.fill(QColor(0,0,0,0));
  QPainter p(&pmap);
  p.setRenderHint(QPainter::RenderHint::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(color);
  p.drawEllipse(pmap.rect());
  QIcon ret(pmap);
  return ret;
}

void ProjectTableRenameDialog::Accept()
{
  if(ui->titleWidget->checkValidity())
    //if data is valid
  {
    WebDialog::Accept();
  }
  else
  {
    Notification::NotifyError(tr("Data is not valid"));
    return;
  }
}
}//namespace Dialogs


void Dialogs::ProjectTableRenameDialog::Open()
{
  WebDialog::Open();
  loadModels();
}
