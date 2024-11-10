#ifndef DND_FILES_CONTAINER_H
#define DND_FILES_CONTAINER_H

#include <QWidget>
#include <QFileInfo>

using FileInfoList = QList<QFileInfo>;

namespace Ui {
class DndFilesContainer;
}

class FilePlateWidget;

enum ContainerType
{
  SingleFile,
  MultiFiles
};

enum KnownFileTypes
{
  FTCdf,
  FTPdf,
  FTXls
};

class DndFilesContainer : public QWidget
{
  Q_OBJECT

public:
  explicit DndFilesContainer(QWidget *parent = nullptr,
                             ContainerType type = SingleFile);
  ~DndFilesContainer();
  void setFrameCaption(QString frameCaption);
  void setAcceptableDrops(QStringList extentions);
  void setFilesDialogFilters(QString filters);
  void setFilesDialogCaption(QString caption);
  FileInfoList getFilesInfoList();

  ContainerType Type() const;
  void setType(ContainerType newType);

  void addFile(const QString& filePath, qint64 fileSize = 0, const QDateTime& date = QDateTime());
  void clear();

  static QString getIconPath(const QFileInfo& fi);

signals:
  void filesAdded(FileInfoList newFilesList);
  void fileRemoved(QFileInfo fi);
  void cleared();

private:
  Ui::DndFilesContainer *ui;
  ContainerType mType;
  QStringList mAcceptableDrops;
  QString mFdFilters;
  QString mFdCaption;
  QList<FilePlateWidget*> mPlates;

  static QMap<QString, KnownFileTypes> mStrToKnownFileType;

private:
  void onLocalImportButtonClicked();
  void onFilesAdded(FileInfoList newFilesList);
  FilePlateWidget *makePlate(const QFileInfo& fi);
  FilePlateWidget *makePlate(const QString& filePath, qint64 fileSize = 0, const QDateTime& date = QDateTime());
  void setupPlate(FilePlateWidget* plate);
};

#endif // DND_FILES_CONTAINER_H
