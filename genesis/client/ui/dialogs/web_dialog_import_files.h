#ifndef WEBDIALOGIMPORTFILES_H
#define WEBDIALOGIMPORTFILES_H

#include <ui/dialogs/templates/info.h>
#include <ui/controls/dnd_zone_frame.h>
#include <ui/controls/file_plate_widget.h>
#include <logic/enums.h>

#include <QMap>
#include <QStack>
#include <QUuid>

class QLabel;
class QStackedWidget;
class QLineEdit;
using namespace Import;

namespace Ui {
class FilesImportDialogBody;
}

namespace Dialogs
{
using Btns = QDialogButtonBox::StandardButton;
using FilePlatesMap = QMap<QString, FilePlateWidget*>;
using FileMzControlsMap = QMap<QString, QLineEdit*>;
using FileMzValuesMap = QMap<QString, QString>;
using FileMzValuesMapPtr = QSharedPointer<FileMzValuesMap>;
using FilesInfoList = QList<QFileInfo>;
using FilesInfoListPtr = QSharedPointer<FilesInfoList>;

class WebDialogImportFiles : public Templates::Info
{
  Q_OBJECT

public:
  enum Page
  {
    PageFilesUpload = 0,
    PageChromatogrammsGroupSetup = 1,
    PageFragmentsMzSetup = 2,
    PagePassport = 3,

    PageLast,
  };
  struct ChromaFile
  {
    QUuid uid;
    QString name;
    ProjectFiles::FileType type;
    QString mz;
    QByteArray data;
  };

private:
  QMap<Page, QString> mTipsMap;
  QStack<Page> mPageStack;
  QString kNextStepText;
  QString kFinishText;
  QString kFinishCancel;
  bool blockCancel = false;

public:
  WebDialogImportFiles(ImportedFilesType type, int projectId, QWidget* parent);
  ~WebDialogImportFiles();

  void Accept() override;
  void Reject() override;

public slots:
  void addFilesToList(FileInfoList paths);
  QPair<int, QString> getGroupInfo();
  bool isAddToGroup();
  FilesInfoListPtr getFiles();
  FileMzValuesMapPtr getFilesMzMap();
  QJsonArray getPassports();

private:
  void setupUi();
  void updateModels();
  void startFilesUploading();
  void uploadFiles(const QList<ChromaFile>& files, int groupId = 0);
  void uploadPassports();

private slots:
  void onPlateClosed(QString absoluteFilePath);
  void stepBack();
  void stepForward();
  void radioButtonInvalidate();
  void InvalidateInput();

  void importFromLocal();
  void allFilesUploadFinished(bool withErrors);

private:
  Ui::FilesImportDialogBody   *ui;
  ImportedFilesType           mFilesType;
  Page                        mCurrentPage;
  FilePlatesMap               mFilePlates;
  FileMzControlsMap           mFileMzMap;

  QPointer<QPushButton>       mBackButton;
  int                         mProjectId;

  QList<ChromaFile> mFilesDataCache;
  int mGroupId = 0;
};
}//namespace Dialogs

#endif // WEBDIALOGIMPORTFILES_H
