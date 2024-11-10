#include "file_plate_widget.h"
#include "ui_file_plate_widget.h"
#include <genesis_style/style.h>
#include <ui/controls/dnd_files_container.h>

FilePlateWidget::FilePlateWidget(const QFileInfo& fileInfo, const QString& fileIcon, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::FilePlateWidget)
{
  ui->setupUi(this);
  setupUi();
  setFileInfo(fileInfo);
  if (!fileIcon.isEmpty())
    ui->label->setPixmap(QPixmap(fileIcon));
}

FilePlateWidget::FilePlateWidget(const QString &fileName, QDateTime lastModified, qint64 fileSize, const QString &fileIcon, QWidget *parent)
  : QWidget(parent),
  ui(new Ui::FilePlateWidget)
{
  ui->setupUi(this);
  setupUi();
  setData(fileName, lastModified, fileSize, fileIcon);
}

FilePlateWidget::FilePlateWidget(QWidget *parent)
  : QWidget(parent),
  ui(new Ui::FilePlateWidget)
{
  ui->setupUi(this);
  setupUi();
}

void FilePlateWidget::setDownloadButtonHidden(bool isDownloadButtonHidden)
{
  ui->loadFileButton->setHidden(isDownloadButtonHidden);
}

FilePlateWidget::~FilePlateWidget()
{
  delete ui;
}

const QFileInfo &FilePlateWidget::fileInfo() const
{
  return m_fileInfo;
}

void FilePlateWidget::setFileInfo(const QFileInfo &newFileInfo)
{
  m_fileInfo = newFileInfo;
  ui->TopLabel->setText(m_fileInfo.fileName());

  auto fileDetails = QString("%1 %2 %3")
                     .arg(QString::asprintf("%.2f", (double)m_fileInfo.size() / 1048576.0))
                     .arg(tr("Mb"))
                     .arg(m_fileInfo.lastModified().toString("dd.MM.yyyy, hh:mm"));
  ui->SubLabel->setText(fileDetails);
}

void FilePlateWidget::setData(const QString &fileName, QDateTime lastModified, qint64 fileSize)
{
  m_fileInfo = QFileInfo(fileName);
  ui->label->setPixmap(QPixmap(DndFilesContainer::getIconPath(m_fileInfo)));
  ui->TopLabel->setText(fileName);

  auto fileDetails = QString("%1 %2 %3")
                       .arg(QString::asprintf("%.2f", (double)fileSize / 1048576.0))
                       .arg(tr("Mb"))
                       .arg(lastModified.toString("dd.MM.yyyy, hh:mm"));
  ui->SubLabel->setText(fileDetails);
}

void FilePlateWidget::setData(const QString &fileName, QDateTime lastModified, qint64 fileSize, const QString &fileIcon)
{
  m_fileInfo = QFileInfo(fileName);
  ui->TopLabel->setText(fileName);

  auto fileDetails = QString("%1 %2 %3")
                       .arg(QString::asprintf("%.2f", (double)fileSize / 1048576.0))
                       .arg(tr("Mb"))
                       .arg(lastModified.toString("dd.MM.yyyy, hh:mm"));
  ui->SubLabel->setText(fileDetails);
  if (!fileIcon.isEmpty())
    ui->label->setPixmap(QPixmap(fileIcon));
}

void FilePlateWidget::setClosable(bool closable)
{
  ui->CancelButton->setHidden(!closable);
}

void FilePlateWidget::setupUi()
{
  connect(ui->loadFileButton, &QPushButton::clicked, this, &FilePlateWidget::downloadButtonClicked);
  setDownloadButtonHidden();

  ui->TopLabel->setStyleSheet(Style::Genesis::Fonts::RegularText());
  QString subStyleSheet =
    "QLabel"
    "  {"
    "  font:   @RegularTextFont;"
    "  color:  @fontColorPrimaryLight;"
    "  }";
  ui->SubLabel->setStyleSheet(Style::ApplySASS(subStyleSheet));
}

void FilePlateWidget::on_CancelButton_clicked()
{
  emit closedByUser(m_fileInfo.absoluteFilePath());
  deleteLater();
}

