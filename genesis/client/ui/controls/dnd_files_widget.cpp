#include "dnd_files_widget.h"
#include "ui_dnd_files_widget.h"
#include "genesis_style/style.h"

#include <QFileDialog>

DnDFilesWidget::DnDFilesWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::DnDFilesWidget)
{
  ui->setupUi(this);
  ui->dndFrame->setStyleSheet(Style::ApplySASS("QFrame#dndFrame { border: 2px dashed @frameColorDarker; border-radius: @borderRadius; }\n"
                                               "QFrame[hover=\"true\"]#dndFrame { background-color: rgba(0, 66, 105, 5%);}"
                                               "QFrame[error=\"true\"]#dndFrame { border: 2px dashed @textColorError;}"
                                               ));
  ui->localImportButton->setStyleSheet("QPushButton {background-color: rgba(0, 66, 105, 7%);}");
  connect(ui->dndFrame, &DnDZoneFrame::filesDropped, this, &DnDFilesWidget::filesAdded);
  connect(ui->localImportButton, &QPushButton::clicked, this, &DnDFilesWidget::onLocalImportButtonClicked);
}

DnDFilesWidget::~DnDFilesWidget()
{
  delete ui;
}

void DnDFilesWidget::setFrameCaption(QString frameCaption)
{
  ui->dndZoneCaption->setText(frameCaption);
}

void DnDFilesWidget::setAcceptableDrops(QStringList extentions)
{
  mAcceptableDrops = extentions;
  ui->dndFrame->setAcceptableFileExtensions(mAcceptableDrops);
}

void DnDFilesWidget::setFilesDialogFilters(QString filters)
{
  mFdFilters = filters;
}

void DnDFilesWidget::setFilesDialogCaption(QString caption)
{
  mFdCaption = caption;
}

void DnDFilesWidget::onLocalImportButtonClicked()
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
    emit filesAdded(finfoList);
  }
}
