#include "project_table_rename_dialog_refactor.h"
#include "ui_project_table_rename_dialog_refactor.h"

#include <api/api_rest.h>
#include <ui/genesis_window.h>
#include <genesis_style/style.h>

#include <QPushButton>
#include <QPainter>

namespace Dialogs
{
ProjectEditDialog::ProjectEditDialog(int projectId,
  const QString& projectTitle,
  const QString& status,
  const QString& comment,
  const QString& codeWord,
  QWidget* parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::ProjectTableRenameDialogRefactor)
  , mContent(new QWidget(nullptr))
  , mProjectId(projectId)
  , mDefaultTitle(projectTitle)
  , mStatus(status)
  , mComment(comment)
  , mCodeWord(codeWord)
{
  setupUi();
}

ProjectEditDialog::~ProjectEditDialog()
{
  delete ui;
}

QString ProjectEditDialog::getTitle()
{
  return ui->widget_title->text();
}

QString ProjectEditDialog::getStatusCode()
{
  return ui->comboBox_status->currentData().toString();
}

QString ProjectEditDialog::getStatus()
{
  return ui->comboBox_status->currentText();
}

QIcon ProjectEditDialog::getStatusIcon()
{
  return ui->comboBox_status->itemIcon(ui->comboBox_status->currentIndex());
}

QString ProjectEditDialog::getComment()
{
  return ui->widget_comment->text();
}

QString ProjectEditDialog::getCodeWord()
{
  return ui->checkBox_codeWord->isChecked() ? ui->widget_codeWord->text() : "";
}

void ProjectEditDialog::setupUi()
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = tr("Common information edit");
  s.buttonsNames = { {QDialogButtonBox::Ok, tr("Save")},
                   {QDialogButtonBox::Cancel, tr("Cancel")} };
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);

  ui->label_status->setStyleSheet(Style::Genesis::Fonts::RegularLight());

  //// Title widget settings
  {
    auto settings = ui->widget_title->settings();
    settings.textHeader = tr("Project title");
    settings.textPlaceholder = tr("Enter title here");
    settings.defaultText = mDefaultTitle;
    settings.controlSymbols = true;
    settings.textTooltipsSet = {
      tr("Title is empty"),
      tr("Title has alredy used"),
      tr("Not checked"),
      tr("Title is valid"),
      tr("Title must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")
    };
    ui->widget_title->applySettings(settings);
  }

  //// Comment widget settings
  {
    auto settings = ui->widget_comment->settings();
    settings.textHeader = tr("Comment");
    settings.textMaxLen = 70;
    settings.textPlaceholder = tr("Enter comment");
    settings.textRequired = false;
    ui->widget_comment->applySettings(settings);
  }

  //// Code word widget settings
  {
    auto settings = ui->widget_codeWord->settings();
    settings.textHeader = tr("Code word");
    settings.textPlaceholder = tr("Enter code word");
    settings.textRequired = true;
    ui->widget_codeWord->applySettings(settings);
  }

  if (!mCodeWord.isEmpty())
    ui->checkBox_codeWord->setChecked(true);
  else
    ui->checkBox_codeWord->setChecked(false);

  setDialogDataValidity();

  connect(ui->widget_title, &DialogLineEditEx::validityChanged, this, &ProjectEditDialog::setDialogDataValidity);
  connect(ui->widget_codeWord, &DialogLineEditEx::validityChanged, this, &ProjectEditDialog::setDialogDataValidity);
  connect(ui->checkBox_codeWord, &QCheckBox::stateChanged, this, [&]
  {
    setDialogDataValidity();
  });
}

void ProjectEditDialog::setDialogDataValidity()
{
  ui->widget_codeWord->setEnabled(ui->checkBox_codeWord->isChecked());

  if (!ui->widget_title->checkValidity() || (ui->checkBox_codeWord->isChecked() && !ui->widget_codeWord->checkValidity()))
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
  else
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
}

void ProjectEditDialog::loadModels()
{
  auto overlayId = GenesisWindow::Get()->ShowOverlay(tr("Loading"));
  //some api request

  {
    auto s = ui->widget_comment->settings();
    s.defaultText = mComment;
    ui->widget_comment->applySettings(s);
  }

  {
    auto s = ui->widget_codeWord->settings();
    s.defaultText = mCodeWord;
    ui->widget_codeWord->applySettings(s);
  }

  auto callback = [overlayId, status = mStatus, this](QNetworkReply*, QJsonDocument doc)
  {
    auto root = doc.object();
    if (root["error"].toBool() == true)
    {
      Notification::NotifyError(root["msg"].toString(), tr("Server error"));
      GenesisWindow::Get()->RemoveOverlay(overlayId);
      Reject();
      return;
    }
    auto jforbiddenArray = root["project_titles"].toArray();
    auto jvariantsArray = root["status_variants"].toArray();

    auto settings = ui->widget_title->settings();
    settings.forbiddenStrings.clear();
    for (const auto& v : jforbiddenArray)
    {
      if (v == settings.defaultText)
        continue;
      settings.forbiddenStrings << v.toString();
    }
    ui->widget_title->applySettings(settings);

    int index{};
    for (size_t i = 0; i < jvariantsArray.size(); i++)
    {
      auto a = jvariantsArray[i].toArray();
      QIcon icon = createCircleIcon({ 10, 10 }, QColor(a[2].toString()));
      ui->comboBox_status->addItem(icon, a[1].toString(), a[0].toString());
      if (a[0].toString() == status)
      {
        index = i;
      }
    }

    ui->comboBox_status->setCurrentIndex(index);

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

QIcon ProjectEditDialog::createCircleIcon(QSize size, QColor color)
{
  QPixmap pmap(size);
  pmap.fill(QColor(0, 0, 0, 0));
  QPainter p(&pmap);
  p.setRenderHint(QPainter::RenderHint::Antialiasing);
  p.setPen(Qt::NoPen);
  p.setBrush(color);
  p.drawEllipse(pmap.rect());
  QIcon ret(pmap);
  return ret;
}

void Dialogs::ProjectEditDialog::Open()
{
  WebDialog::Open();
  loadModels();
}
}
