#include "web_dialog_import_files.h"
#include "ui_web_dialog_import_files.h"
#include <genesis_style/style.h>
#include <api/api_rest.h>
#include <logic/notification.h>
#include <logic/context_root.h>
#include <logic/tree_model.h>
#include <logic/known_context_tag_names.h>
#include "logic/known_json_tag_names.h"

#include <QComboBox>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QDialog>
#include <QFileDialog>

using namespace Import;
using namespace Dialogs;
using namespace Core;

namespace ImportDetails
{
const int MaximumFiles = 30;

QString GetTemplateCounter()
{
  return Style::ApplySASS("<span style=\"color: @brandColorDarker;\">%1</span>");
}

QString GetTemplateHint()
{
  return Style::ApplySASS("<span style=\"color: @textColorPale;\"><b>%1</b></span>");
}

QString GetLineEditStyle()
{
  return "QLineEdit{color: none;}";
}

QString GetFileFrameStyle()
{
  return Style::ApplySASS("QFrame#dndFrame { border: 2px dashed @frameColorDarker; border-radius: @borderRadius; }\n"
                          "QFrame[error=\"true\"]#dndFrame { border: 2px dashed @textColorError;   border-radius: @borderRadius; }"
                          );
}
QString GetButtonStyle()
{
  return Style::ApplySASS("QPushButton {"
                          "outline:            transparent;\n"
                          "font:               @defaultFont;\n"
                          "padding:            @pushButtonPaddingVScalablepx @pushButtonPaddingHScalablepx @pushButtonPaddingVScalablepx @pushButtonPaddingHScalablepx;\n"
                          "color:              @uiButtonColorPrimaryFg;\n"
                          "background-color:   @uiButtonColorPrimaryBg;\n"
                          "border-color:       @uiButtonColorPrimaryBg;\n"
                          "border-width:       1px;\n"
                          "border-radius:      @borderRadius;\n"
                          "border-style:       solid;}");
}
}

WebDialogImportFiles::WebDialogImportFiles(ImportedFilesType type, int projectId, QWidget* parent)
  : Templates::Info(parent, Btns::Ok | Btns::Cancel)
  , ui(new Ui::FilesImportDialogBody)
  , mFilesType(type)
  , mCurrentPage(PageFilesUpload)
  , mProjectId(projectId)
{
  mTipsMap = {{PageFilesUpload, tr("step 1/3. Loading")},
              {PageChromatogrammsGroupSetup, tr("step 2/3. Import setup")},
              {PageFragmentsMzSetup, tr("step 2/3. m/z assigning")},
              {PagePassport, tr("step 3/3. Passport setup")},
              {PageLast, "ERROR"}};
  kNextStepText = tr("Next step");
  kFinishText = tr("Import");
  kFinishCancel = tr("Don't create passports");

  auto s = Templates::Info::Settings();
  s.dialogHeader = tr("Chromatogramms import");
  s.contentHeader = mTipsMap[mCurrentPage];
  s.buttonsNames = {{Btns::Ok, kNextStepText}};
  s.buttonsProperties = {{Btns::Ok, {{"blue", true}}}};

  setupUi();
  applySettings(s);
}

WebDialogImportFiles::~WebDialogImportFiles()
{
  delete ui;
}

void WebDialogImportFiles::Accept()
{
  switch(mCurrentPage)
  {
  case PagePassport:
    if(!ui->passportPage->allChromasHasFileId())
    {
      Notification::NotifyError(tr("Not all files are uploaded to the server, can't proceed"), tr("Uploading error"));
      break;
    }
    if(!ui->passportPage->CheckValidity())
    {
      Notification::NotifyError(tr("Not all required fields are completed"), tr("Completion error"));
      break;
    }
    uploadPassports();
    blockCancel = true;
    break;
  case PageFilesUpload:
  case PageChromatogrammsGroupSetup:
  case PageFragmentsMzSetup:
    stepForward();
    break;
  default:
    break;
  }
}

void WebDialogImportFiles::Reject()
{
  if(blockCancel)
    return;
  TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
  TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
  switch(mCurrentPage)
  {
  case PagePassport:
    if(!ui->passportPage->allChromasHasFileId())
      Notification::NotifyError(tr("Not all files are uploaded to the server, can't proceed"), tr("Uploading error"));
    else
      Templates::Info::Reject();
    break;
  default:
    Templates::Info::Reject();
    break;
  }
}

//void ThisDial::Reject()
//{

//}

void WebDialogImportFiles::stepBack()
{
  if(mPageStack.isEmpty())
    return;
  mCurrentPage = mPageStack.pop();
  ui->stackedWidget->setCurrentIndex((int)mCurrentPage);
  mBackButton->setVisible(false);
  auto s = getCurrentSettings();
  s.contentHeader = mTipsMap[mCurrentPage];
  s.buttonsNames[Btns::Ok] = kNextStepText;
  applySettings(s);
  InvalidateInput();
}
void WebDialogImportFiles::stepForward()
{
  auto updateUi = [this](Page newPage, Page prevPage)
  {
    mCurrentPage = newPage;
    ui->stackedWidget->setCurrentIndex((int)mCurrentPage);
    mBackButton->setVisible(!mPageStack.isEmpty());
    auto s = getCurrentSettings();
    s.contentHeader = mTipsMap[mCurrentPage];
    if(mCurrentPage == PagePassport)
    {
      s.buttonsNames[Btns::Ok] = kFinishText;
      s.buttonsNames[Btns::Cancel] = kFinishCancel;
    }
    else
    {
      s.buttonsNames[Btns::Ok] = kNextStepText;
      s.buttonsNames.remove(Btns::Cancel);
    }
    applySettings(s);
    InvalidateInput();
  };
  switch(mCurrentPage)
  {
  case PageFilesUpload:
  {
    using namespace Import;
    switch(mFilesType)
    {
    case Chromatogramms:
      mPageStack.push(PageFilesUpload);
      updateUi(PageChromatogrammsGroupSetup, PageFilesUpload);
      break;
    case Fragments:
      mPageStack.push(PageFilesUpload);
      updateUi(PageFragmentsMzSetup, PageFilesUpload);
      break;
    default:
      break;
    }
    break;
  }
  case PageChromatogrammsGroupSetup:
  {
    mPageStack.clear();
    updateUi(PagePassport, PageChromatogrammsGroupSetup);
    ui->passportPage->loadComboModels(false);
    startFilesUploading();
    break;
  case PageFragmentsMzSetup:
    mPageStack.clear();
    updateUi(PagePassport, PageFragmentsMzSetup);
    ui->passportPage->loadComboModels(true);
    startFilesUploading();
    break;
  case PagePassport:
  case PageLast:
    break;
  }
  }
}

void WebDialogImportFiles::radioButtonInvalidate()
{
  ui->groupsCombo->setVisible(ui->toGroupRadio->isChecked());
  InvalidateInput();
}

void WebDialogImportFiles::InvalidateInput()
{
  auto fragmentsMzIsValid = [this](){
    for(auto& edit : mFileMzMap)
      if(edit->text().isEmpty())
        return false;
    return true;
  };

  bool isValid = false;
  switch(mCurrentPage)
  {
  case PageFilesUpload:
    isValid = !mFilePlates.isEmpty();
    break;
  case PageChromatogrammsGroupSetup:
    isValid = (!ui->toGroupRadio->isChecked() ||
               !ui->groupsCombo->currentText().isEmpty());
    break;
  case PageFragmentsMzSetup:
    isValid = fragmentsMzIsValid();
    break;
  case PagePassport:
    isValid = ui->passportPage->allChromasHasFileId();
    break;
  case PageLast:
    break;
  }
  ButtonBox->button(Btns::Ok)->setEnabled(isValid);
}

void WebDialogImportFiles::addFilesToList(FileInfoList infoList)
{
  for(const auto &info : infoList)
  {
    if(!mFilePlates.contains(info.absoluteFilePath()))
    {
      auto plate = new FilePlateWidget(info);
      mFilePlates[info.absoluteFilePath()] = plate;
      ui->filesListLayout->addWidget(plate);
      connect(plate, &FilePlateWidget::closedByUser, this, &WebDialogImportFiles::onPlateClosed);

      auto mzEdit = new QLineEdit();
      mzEdit->setStyleSheet(ImportDetails::GetLineEditStyle());
      mzEdit->setPlaceholderText(tr("Specify m/z"));
      ui->fragmentsMzLayout->addRow(info.fileName(), mzEdit);
      connect(mzEdit, &QLineEdit::textEdited, this, &WebDialogImportFiles::InvalidateInput);

      auto pal = mzEdit->palette();
      pal.setColor(QPalette::PlaceholderText, Qt::lightGray);
      mzEdit->setPalette(pal);
      mFileMzMap[info.absoluteFilePath()] = mzEdit;
    }
  }
  InvalidateInput();
}

void WebDialogImportFiles::setupUi()
{
  auto body = new QWidget(this);
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  ui->setupUi(body);
  getContent()->layout()->addWidget(body);
  ui->fragmentMzColumnTitle->setText(tr("m/z") + QString(" %1").arg(Style::GetInputAlert()));
  ui->dndFrame->setStyleSheet(ImportDetails::GetFileFrameStyle());
  ui->localImportButton->setStyleSheet(ImportDetails::GetButtonStyle());
  connect(ui->localImportButton, &QPushButton::clicked, this, &WebDialogImportFiles::importFromLocal);
  ui->systemImportButton->setStyleSheet(ImportDetails::GetButtonStyle());
  connect(ui->dndFrame, &DnDZoneFrame::filesDropped, this, &WebDialogImportFiles::addFilesToList);

  mBackButton = new QPushButton(body);
  mBackButton->setText(tr("Previous step"));
  mBackButton->setIcon(QIcon("://resource/icons/icon_button_lt.png"));
  mBackButton->setStyleSheet(Style::Genesis::GetUiStyle());
  mBackButton->setProperty("secondary", true);
  mBackButton->setVisible(false);

  ButtonLayout->insertSpacerItem(0, new QSpacerItem(0,0,QSizePolicy::Expanding));
  ButtonLayout->insertWidget(0, mBackButton);
  connect(mBackButton, &QPushButton::clicked, this, &WebDialogImportFiles::stepBack);

  connect(ui->toGroupRadio, &QRadioButton::toggled, this, &WebDialogImportFiles::radioButtonInvalidate);
  radioButtonInvalidate();

  ui->stackedWidget->setCurrentIndex(0);

  auto palette = ui->groupsCombo->lineEdit()->palette();
  palette.setColor(QPalette::PlaceholderText, Qt::lightGray);
  ui->groupsCombo->lineEdit()->setPalette(palette);
  ui->groupsCombo->lineEdit()->setPlaceholderText(tr("Type new group name or pick group from list"));

  connect(ui->groupsCombo, &QComboBox::currentTextChanged, this, &WebDialogImportFiles::InvalidateInput);

  updateModels();
  Size = QSizeF(0.8, 0.8);
  UpdateGeometry();

  connect(ui->passportPage, &PassportMainForm::reloadChroma, this, [this](QUuid chromaUid)
          {
            ButtonBox->button(Btns::Ok)->setEnabled(false);
            ButtonBox->button(Btns::Cancel)->setEnabled(false);
            for(auto& f : mFilesDataCache)
            {
              if(f.uid == chromaUid)
              {
                uploadFiles({f}, mGroupId);
                return;
              }
            }
            Notification::NotifyWarning(tr("Uploading file data is not exists on WebDialogImportFiles. Just nothing happens."), tr("Warning"));
          });
  connect(ui->passportPage, &PassportMainForm::allFilesUploadFinished, this, &WebDialogImportFiles::allFilesUploadFinished);
}

void WebDialogImportFiles::updateModels()
{
  API::REST::GetFileGroups(mProjectId, [this](QNetworkReply* r, QJsonDocument doc)
  {
    ui->groupsCombo->clear();
    auto jdata = doc.object()["data"].toArray();
    for(int i = 0; i < jdata.size(); i++)
    {
      auto jitem = jdata[i].toArray();
      ui->groupsCombo->addItem(jitem[1].toString(), jitem[0].toInt());
    }
    ui->groupsCombo->setCurrentIndex(-1);

  },
  [](QNetworkReply* r, QNetworkReply::NetworkError err)
  {
    Notification::NotifyError(tr("Failed to load file groups information"), err);
  });
}

void WebDialogImportFiles::startFilesUploading()
{
  auto filesList = getFiles();
  switch(mFilesType)
  {
  case Import::Chromatogramms:
  {
    for(const auto& info : *filesList)
    {
      QFile file(info.absoluteFilePath());
      file.open(QIODevice::ReadOnly);
      ChromaFile cf;
      cf.name = info.fileName();
      cf.type = ProjectFiles::FileType::tic;
      cf.uid = QUuid::createUuid();
      cf.mz = "";
      cf.data = file.readAll();
      mFilesDataCache << cf;
      file.close();
    }
    if(isAddToGroup())
    {
      auto groupInfo = getGroupInfo();
      if(groupInfo.first == 0)
      {
        API::REST::AddFileGroup(mProjectId, groupInfo.second,
                                 [this](QNetworkReply* r, QJsonDocument doc){
          mGroupId = doc.object()["data"].toArray().first().toArray().first().toInt();

          QMap<QUuid, QString> filesNames;
          for(auto& f : mFilesDataCache)
            filesNames[f.uid] = f.name;
          ui->passportPage->setChromatogrammsList(filesNames);
          uploadFiles(mFilesDataCache, mGroupId);
        },
        [groupInfo](QNetworkReply* r, QNetworkReply::NetworkError error){
          Notification::NotifyError(tr("Failed to create new file group") + QString(" %1").arg(groupInfo.second), tr("Upload error"));
        });
      }
      else
      {
        mGroupId = groupInfo.first;
        QMap<QUuid, QString> filesNames;
        for(auto& f : mFilesDataCache)
          filesNames[f.uid] = f.name;
        ui->passportPage->setChromatogrammsList(filesNames);
        uploadFiles(mFilesDataCache, mGroupId);
      }
    }
    else
    {
      QMap<QUuid, QString> filesNames;
      for(auto& f : mFilesDataCache)
        filesNames[f.uid] = f.name;
      ui->passportPage->setChromatogrammsList(filesNames);
      uploadFiles(mFilesDataCache);
    }
    break;
  }
  case Import::Fragments:
  {
    auto mzMap = getFilesMzMap();
    for(const auto& info : *filesList)
    {
      QFile file(info.absoluteFilePath());
      file.open(QIODevice::ReadOnly);
      QString ion_mass = (*mzMap)[info.absoluteFilePath()];
      ChromaFile cf;
      cf.name = info.fileName();
      cf.mz = ion_mass;
      cf.type = ProjectFiles::FileType::sim;
      cf.uid = QUuid::createUuid();
      cf.data = file.readAll();
      mFilesDataCache << cf;
      file.close();
    }
    QMap<QUuid, QString> filesNames;
    for(auto& f : mFilesDataCache)
      filesNames[f.uid] = f.name;
    ui->passportPage->setChromatogrammsList(filesNames);
    uploadFiles(mFilesDataCache);
    break;
  }
  default:
    break;
  }
}

void WebDialogImportFiles::uploadFiles(const QList<ChromaFile> &files, int groupId)
{

  ButtonBox->button(Btns::Ok)->setEnabled(false);
  ButtonBox->button(Btns::Cancel)->setEnabled(false);
  auto handleFileUpload = [this](QNetworkReply* r, QJsonDocument doc, QUuid uid, QString name)
  {
    auto obj = doc.object();
    if(obj["error"].toBool())
    {
      Notification::NotifyError(tr("Failed to import file \"%1\"; %2").arg(name).arg(obj["msg"].toString()), tr("Upload error"));
      ui->passportPage->updateChromaLoadingPrecentage(uid, -1);
      ui->passportPage->updateChromaFileId(uid, -1);
      return;
    }
    ui->passportPage->updateChromaFileId(uid, obj["id"].toInt());
    ui->passportPage->updateChromaLoadingPrecentage(uid, 100);
//    InvalidateInput();
    Notification::NotifySuccess(tr("File successfully imported").arg(name), name);
  };
  auto handleFileUploadError = [this](QNetworkReply* r, QNetworkReply::NetworkError error, QUuid uid, QString name)
  {
    ui->passportPage->updateChromaFileId(uid, -1);
    ui->passportPage->updateChromaLoadingPrecentage(uid, -1);
    Notification::NotifyError(tr("Failed to import file \"%1\"").arg(name), tr("Upload error"), error);
  };

  for (auto& f : files)
  {
    API::REST::UploadFile(f.name, mProjectId, f.type, f.data, groupId, f.mz,
    [f, handleFileUpload](QNetworkReply* r, QJsonDocument doc){handleFileUpload(r, doc, f.uid, f.name);},
    [f, handleFileUploadError](QNetworkReply* r, QNetworkReply::NetworkError error){handleFileUploadError(r, error, f.uid, f.name);},
    [this, f](qint64 bytesSent, qint64 bytesTotal)
    {
      if(bytesTotal == 0)
        return;
      int precentage = (double(bytesSent) / (double(bytesTotal) / 100.0) + 0.5);
      if(precentage == 100)
        precentage = 99;
      ui->passportPage->updateChromaLoadingPrecentage(f.uid, precentage);
    }
    );
  }
}

void WebDialogImportFiles::uploadPassports()
{
  Notification::NotifyInfo(tr("Passports uploading started"), tr("Information"));
  QJsonObject obj;
  obj.insert(JsonTagNames::project_id, GenesisContextRoot::Get()->ProjectId());
  obj.insert(JsonTagNames::ChromatogrammData, ui->passportPage->toJson());
  //qDebug().noquote() << QJsonDocument(data).toJson();
  API::REST::SavePassportV2(obj,
                          [this](QNetworkReply* r, QJsonDocument doc)
  {
    auto obj = doc.object();
    if(obj["error"].toBool())
    {
      Notification::NotifyError(tr("Failed to upload passports: %1").arg(obj["msg"].toString()), tr("Upload error"));
      blockCancel = false;
      return;
    }
    Notification::NotifySuccess(tr("Passports successfully uploaded"), tr("Success"));
    blockCancel = false;
    TreeModel::ResetInstances("TreeModelDynamicProjectChromatogramms");
    TreeModel::ResetInstances("TreeModelDynamicProjectFragments");
    Done(QDialog::Accepted);
  },
  [this](QNetworkReply* r, QNetworkReply::NetworkError error)
  {
    Notification::NotifyError(tr("Failed to upload passports"), tr("Network error"), error);
    blockCancel = false;
  },
  Qt::QueuedConnection);

}

void WebDialogImportFiles::onPlateClosed(QString absoluteFilePath)
{
  mFilePlates.remove(absoluteFilePath);
  ui->fragmentsMzLayout->removeRow(mFileMzMap[absoluteFilePath]);
  mFileMzMap.remove(absoluteFilePath);
  InvalidateInput();
}

QPair<int, QString> WebDialogImportFiles::getGroupInfo()
{
  auto data = ui->groupsCombo->currentData();
  auto currentDataString = ui->groupsCombo->currentData(Qt::EditRole).toString();
  if(currentDataString != ui->groupsCombo->currentText())
    return {0, ui->groupsCombo->currentText()};
  if(data.isValid() && !data.isNull())
    return {data.toInt(), QString()};
  else
    return {0, ui->groupsCombo->currentText()};
}

bool WebDialogImportFiles::isAddToGroup()
{
  return ui->toGroupRadio->isChecked();
}

FilesInfoListPtr WebDialogImportFiles::getFiles()
{
  FilesInfoListPtr list(new FilesInfoList());
  for(const auto& plate : qAsConst(mFilePlates))
    list->append(plate->fileInfo());
  return list;
}

FileMzValuesMapPtr WebDialogImportFiles::getFilesMzMap()
{
  FileMzValuesMapPtr map(new FileMzValuesMap());
  for(auto& key : mFileMzMap.keys())
  {
    (*map)[key] = mFileMzMap[key]->text();
  }
  return map;
}

QJsonArray WebDialogImportFiles::getPassports()
{
  return ui->passportPage->toJson();
}

void WebDialogImportFiles::importFromLocal()
{
  auto files = QFileDialog::getOpenFileNames(this, tr("Pick files to import"), "","Curve data file (*.cdf)");
  FileInfoList infoList;

  for(const auto& file : files)
    infoList << QFileInfo(file);

  addFilesToList(infoList);
}

void WebDialogImportFiles::allFilesUploadFinished(bool withErrors)
{
  ButtonBox->button(Btns::Ok)->setEnabled(!withErrors);
  ButtonBox->button(Btns::Cancel)->setEnabled(!withErrors);
}
