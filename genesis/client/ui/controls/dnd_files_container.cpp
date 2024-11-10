#include "dnd_files_container.h"
#include "ui_dnd_files_container.h"
#include "genesis_style/style.h"
#include "ui/controls/file_plate_widget.h"

#include <QFileDialog>

QMap<QString, KnownFileTypes> DndFilesContainer::mStrToKnownFileType = {{"cdf", FTCdf},
                                                                        {"pdf", FTPdf},
                                                                        {"xls", FTXls},
                                                                        {"xlsx", FTXls}};

DndFilesContainer::DndFilesContainer(QWidget *parent,
                                     ContainerType type) :
  QWidget(parent),
  ui(new Ui::DndFilesContainer)
{
  mType = type;
  ui->setupUi(this);
  ui->dndFrame->setStyleSheet(Style::ApplySASS("QFrame#dndFrame { border: 2px dashed @frameColorDarker; border-radius: @borderRadius; }\n"
                                               "QFrame[hover=\"true\"]#dndFrame { background-color: rgba(0, 66, 105, 5%);}"
                                               "QFrame[error=\"true\"]#dndFrame { border: 2px dashed @textColorError;}"
                                               ));
  ui->localImportButton->setStyleSheet("QPushButton {background-color: rgba(0, 66, 105, 7%);}");
  connect(ui->dndFrame, &DnDZoneFrame::filesDropped, this, &DndFilesContainer::onFilesAdded);
  connect(ui->localImportButton, &QPushButton::clicked, this, &DndFilesContainer::onLocalImportButtonClicked);
}

DndFilesContainer::~DndFilesContainer()
{
  delete ui;
}

void DndFilesContainer::setFrameCaption(QString frameCaption)
{

  ui->dndZoneCaption->setText(frameCaption);
}

void DndFilesContainer::setAcceptableDrops(QStringList extentions)
{
  mAcceptableDrops = extentions;
  ui->dndFrame->setAcceptableFileExtensions(mAcceptableDrops);
}

void DndFilesContainer::setFilesDialogFilters(QString filters)
{
  mFdFilters = filters;
}

void DndFilesContainer::setFilesDialogCaption(QString caption)
{
  mFdCaption = caption;
}

FileInfoList DndFilesContainer::getFilesInfoList()
{
  FileInfoList list;
  for(auto& p : mPlates)
    list << p->fileInfo();
  return list;
}

ContainerType DndFilesContainer::Type() const
{
  return mType;
}

void DndFilesContainer::setType(ContainerType newType)
{
  mType = newType;
}

void DndFilesContainer::addFile(const QString &filePath, qint64 fileSize, const QDateTime &date)
{
  if(mType == SingleFile)
    clear();
  auto plate = makePlate(filePath, fileSize, date);
  ui->platesListLayout->addWidget(plate);
  emit filesAdded({plate->fileInfo()});
}

void DndFilesContainer::clear()
{
  for(auto& w : mPlates)
    delete w;
  mPlates.clear();
}

QString DndFilesContainer::getIconPath(const QFileInfo &fi)
{
  auto extention = fi.suffix();
  QString iconPath;
  if(!mStrToKnownFileType.contains(extention))
    iconPath = ":/resource/icons/blank_file.png";
  auto knownFileType = mStrToKnownFileType[extention];
  switch(knownFileType)
  {
  case FTCdf:
    iconPath = ":/resource/icons/icon_cdf_file.png";
    break;
  case FTPdf:
    iconPath = ":/resource/icons/pdf@2x.png";
    break;
  case FTXls:
    iconPath = ":/resource/icons/xls@2x.png";
    break;
    break;
  }
  return iconPath;
}

void DndFilesContainer::onLocalImportButtonClicked()
{
  auto files = QFileDialog::getOpenFileNames(this, mFdCaption, "", mFdFilters);
  if (!files.isEmpty())
  {
    FileInfoList finfoList;
    for(auto& f : files)
    {
      if(!f.isEmpty())
        finfoList << QFileInfo(f);
    }
    onFilesAdded(finfoList);
  }
}

void DndFilesContainer::onFilesAdded(FileInfoList newFilesList)
{
  if(newFilesList.isEmpty())
    return;
  clear();

  if(mType == SingleFile)
    newFilesList = {newFilesList.first()};
  for(auto& fi : newFilesList)
    ui->platesListLayout->addWidget(makePlate(fi));
  emit filesAdded(newFilesList);
}

FilePlateWidget* DndFilesContainer::makePlate(const QFileInfo &fi)
{
  auto plate = new FilePlateWidget(fi, getIconPath(fi), this);
  setupPlate(plate);
  return plate;
}

FilePlateWidget *DndFilesContainer::makePlate(const QString &filePath, qint64 fileSize, const QDateTime &date)
{
  auto plate = new FilePlateWidget(filePath, date, fileSize, getIconPath(QFileInfo(filePath)));
  setupPlate(plate);
  return plate;
}

void DndFilesContainer::setupPlate(FilePlateWidget *plate)
{
  connect(plate, &FilePlateWidget::closedByUser, this,
          [this, plate]()
          {
            mPlates.removeOne(sender());
            emit fileRemoved(plate->fileInfo());
            if(mPlates.isEmpty())
              emit cleared();
          });
  mPlates << plate;
}
