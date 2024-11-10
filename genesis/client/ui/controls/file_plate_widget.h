#ifndef FILEPLATEWIDGET_H
#define FILEPLATEWIDGET_H

#include <QWidget>
#include <QFileInfo>

namespace Ui {
class FilePlateWidget;
}

class FilePlateWidget : public QWidget
{
  Q_OBJECT

public:
  explicit FilePlateWidget(const QFileInfo& fileInfo, const QString& fileIcon = "", QWidget *parent = nullptr);
  FilePlateWidget(const QString& fileName, QDateTime lastModified, qint64 fileSize, const QString& fileIcon = "", QWidget *parent = nullptr);
  FilePlateWidget(QWidget *parent = nullptr);
  void setDownloadButtonHidden(bool isDownloadButtonHidden = true);
  ~FilePlateWidget();

  const QFileInfo &fileInfo() const;
  void setFileInfo(const QFileInfo &newFileInfo);
  void setData(const QString& fileName, QDateTime lastModified, qint64 fileSize);
  void setData(const QString& fileName, QDateTime lastModified, qint64 fileSize, const QString& fileIcon);
  void setClosable(bool closable = true);

private:
  void setupUi();

signals:
  void closedByUser(QString absoluteFilePath);
  void downloadButtonClicked();

private slots:
  void on_CancelButton_clicked();

private:
  Ui::FilePlateWidget *ui;
  QFileInfo m_fileInfo;
};

#endif // FILEPLATEWIDGET_H
