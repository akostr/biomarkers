#pragma once

#include "web_dialog.h"

#include "../controls/dnd_files.h"

#include <QLabel>
#include <QStackedWidget>

/////////////////////////////////////////////////////
//// Web Dialog / upload files
class WebDialogUploadFiles : public WebDialog
{
  Q_OBJECT

public:
  enum PageId
  {
    PageIdPrimary,
    PageIdSecondary,

    PageIdLast,
  };

public:
  WebDialogUploadFiles(QWidget* parent);
  ~WebDialogUploadFiles();

  virtual void Accept() override;
  virtual void Reject() override;

  void SetPage(PageId pageId);
  PageId GetPage();

private:
  void SetupModels();
  void SetupUi();

private slots:
  void UpdateButtons();

private:
  //// Models

  //// Ui
  QPointer<QWidget>         Body;
  QPointer<QVBoxLayout>     BodyLayout;

  //// Pages
  QPointer<QStackedWidget>  Pages;

  //// Pages / primary
  QPointer<QWidget>         PagePrimary;
  QPointer<QVBoxLayout>     PagePrimaryLayout;
  QPointer<QLabel>          PagePrimaryCounter;
  QPointer<QWidget>         PagePrimaryBody;
  QPointer<QVBoxLayout>     PagePrimaryBodyLayout;
  QPointer<DnDFiles>        PagePrimaryFiles;
  QPointer<QLabel>          PagePrimaryError;

  //// Pages / secondary
  QPointer<QWidget>         PageSecondary;
  QPointer<QVBoxLayout>     PageSecondaryLayout;
  QPointer<QLabel>          PageSecondaryCounter;
  QPointer<QWidget>         PageSecondaryBody;
  QPointer<QVBoxLayout>     PageSecondaryBodyLayout;
  QPointer<DnDFiles>        PageSecondaryFiles;
  QPointer<QLabel>          PageSecondaryError;
};
