#include "web_dialog_upload_files.h"

#include "../../api/api_rest.h"
#include "../../logic/context_root.h"
#include "../../logic/tree_model_dynamic_file_types.h"
#include "../../logic/notification.h"
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>
#include <genesis_style/style.h>
#include <ui/genesis_window.h>
#include "ui/known_view_names.h"

#include <QComboBox>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QDebug>

using namespace Core;
using namespace Names;
using namespace ViewPageNames;

namespace Details
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

  QString GetFileFrameStyle()
  {
    return Style::ApplySASS(
      "QWidget#dndframe                 { border: 2px dashed @frameColorDarker; border-radius: @borderRadius; }"
      "QWidget[error=\"true\"]#dndframe { border: 2px dashed @textColorError;   border-radius: @borderRadius; }"

    );
  }
}

/////////////////////////////////////////////////////
//// Web Dialog / upload files
WebDialogUploadFiles::WebDialogUploadFiles(QWidget* parent)
  : WebDialog(parent, nullptr, QSizeF(0.4, 0), QDialogButtonBox::Apply | QDialogButtonBox::Cancel)
{
  SetupModels();
  SetupUi();
}

WebDialogUploadFiles::~WebDialogUploadFiles()
{
}

void WebDialogUploadFiles::SetupModels()
{
}

void WebDialogUploadFiles::SetupUi()
{
  //// Body
  Body = new QWidget(Content);
  Content->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  BodyLayout->setSpacing(0);

  //// Pages
  Pages = new QStackedWidget(Body);
  BodyLayout->addWidget(Pages);

  Pages->setContentsMargins(0, 0, 0, 0);

  //// Pages / primary
  {
    PagePrimary = new QWidget(Pages);
    Pages->addWidget(PagePrimary);

    PagePrimaryLayout = new QVBoxLayout(PagePrimary);
    PagePrimaryLayout->setContentsMargins(0, 0, 0, 0);

    //// Content
    {
      //// Header
      {
        QHBoxLayout* captionLayout = new QHBoxLayout;
        PagePrimaryLayout->addLayout(captionLayout);

        captionLayout->setContentsMargins(0, 0, 0, 0);

        //// Caption
        {
          QLabel* caption = new QLabel(tr("Import files"), PagePrimary);
          captionLayout->addWidget(caption);

          caption->setAlignment(Qt::AlignCenter);
          caption->setStyleSheet(Style::Genesis::GetH2());
        }

        //// Counter
        {
          PagePrimaryCounter = new QLabel(Details::GetTemplateCounter().arg(QString("0/%1").arg(Details::MaximumFiles)), PagePrimary);
          captionLayout->addWidget(PagePrimaryCounter, 0, Qt::AlignBottom);

          PagePrimaryCounter->setStyleSheet(Style::Genesis::GetH2());
          PagePrimaryCounter->setContentsMargins(6, 6, 6, 6);
        }

        //// Space
        captionLayout->addStretch();

        //// Hint
//        {
//          QLabel* hint = new QLabel(Details::GetTemplateHint().arg(tr("Step 1 of 2. Samples")));
//          captionLayout->addWidget(hint, 0, Qt::AlignBottom);

//          hint->setContentsMargins(6, 6, 6, 6);
//        }
      }

      //// Space
      PagePrimaryLayout->addSpacing(Style::Scale(24));

      //// Input
      {
        PagePrimaryBody = new QWidget(PagePrimary);
        PagePrimaryLayout->addWidget(PagePrimaryBody);

        PagePrimaryBody->setMinimumSize(400, 400);
        PagePrimaryBody->setObjectName("dndframe");
        PagePrimaryBody->setStyleSheet(Details::GetFileFrameStyle());

        PagePrimaryBodyLayout = new QVBoxLayout(PagePrimaryBody);
        PagePrimaryBodyLayout->setContentsMargins(Style::Scale(24), Style::Scale(24), Style::Scale(24), Style::Scale(24));

        //// Drop area
        {
          //// Files
          PagePrimaryFiles = new DnDFiles(PagePrimaryBody);
          PagePrimaryBodyLayout->addWidget(PagePrimaryFiles);

          auto handleFiles = [this]()
          {
            PagePrimaryCounter->setText(Details::GetTemplateCounter().arg(QString("%1/%2")
                .arg(PagePrimaryFiles->GetFilesCount())
                .arg(Details::MaximumFiles)));
            UpdateButtons();
          };
          connect(PagePrimaryFiles, &DnDFiles::FilesChanged, handleFiles);
        }
      }

      //// Error
      {
        PagePrimaryError = new QLabel(PagePrimary);
        PagePrimaryLayout->addWidget(PagePrimaryError);

        PagePrimaryError->setStyleSheet(Style::ApplySASS("QLabel { color: @textColorError; }"));
      }
    }
  }

//  //// Pages / secondary
//  {
//    PageSecondary = new QWidget(Pages);
//    Pages->addWidget(PageSecondary);

//    PageSecondaryLayout = new QVBoxLayout(PageSecondary);
//    PageSecondaryLayout->setContentsMargins(0, 0, 0, 0);

//    //// Content
//    {
//      //// Header
//      {
//        QHBoxLayout* captionLayout = new QHBoxLayout;
//        PageSecondaryLayout->addLayout(captionLayout);

//        captionLayout->setContentsMargins(0, 0, 0, 0);

//        //// Caption
//        {
//          QLabel* caption = new QLabel(tr("Import files"), PageSecondary);
//          captionLayout->addWidget(caption);

//          caption->setAlignment(Qt::AlignCenter);
//          caption->setStyleSheet(Style::Genesis::GetH2());
//        }

//        //// Counter
//        {
//          PageSecondaryCounter = new QLabel(Details::GetTemplateCounter().arg(QString("0/%1").arg(Details::MaximumFiles)), PageSecondary);
//          captionLayout->addWidget(PageSecondaryCounter, 0, Qt::AlignBottom);

//          PageSecondaryCounter->setStyleSheet(Style::Genesis::GetH2());
//          PageSecondaryCounter->setContentsMargins(6, 6, 6, 6);
//        }

//        //// Space
//        captionLayout->addStretch();

//        //// Hint
//        {
//          QLabel* hint = new QLabel(Details::GetTemplateHint().arg(tr("Step 2 of 2. Auxiliary")));
//          captionLayout->addWidget(hint, 0, Qt::AlignBottom);

//          hint->setContentsMargins(6, 6, 6, 6);
//        }
//      }

//      //// Space
//      PageSecondaryLayout->addSpacing(Style::Scale(24));

//      //// Input
//      {
//        PageSecondaryBody = new QWidget(PageSecondary);
//        PageSecondaryLayout->addWidget(PageSecondaryBody);

//        PageSecondaryBody->setMinimumSize(400, 400);
//        PageSecondaryBody->setObjectName("dndframe");
//        PageSecondaryBody->setStyleSheet(Details::GetFileFrameStyle());

//        PageSecondaryBodyLayout = new QVBoxLayout(PageSecondaryBody);
//        PageSecondaryBodyLayout->setContentsMargins(Style::Scale(24), Style::Scale(24), Style::Scale(24), Style::Scale(24));

//        //// Drop area
//        {
//          //// Files
//          PageSecondaryFiles = new DnDFiles(PageSecondaryBody);
//          PageSecondaryBodyLayout->addWidget(PageSecondaryFiles);

//          auto handleFiles = [this]()
//          {
//            PageSecondaryCounter->setText(Details::GetTemplateCounter().arg(QString("%1/%2")
//              .arg(PageSecondaryFiles->GetFilesCount())
//              .arg(Details::MaximumFiles)));
//            UpdateButtons();
//          };
//          connect(PageSecondaryFiles, &DnDFiles::FilesChanged, handleFiles);
//        }
//      }

//      //// Error
//      {
//        PageSecondaryError = new QLabel(PageSecondary);
//        PageSecondaryLayout->addWidget(PageSecondaryError);

//        PageSecondaryError->setStyleSheet(Style::ApplySASS("QLabel { color: @textColorError; }"));
//      }
//    }
//  }

  //// Buttons
  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setDefault(true);

    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  //// Set page
  SetPage(PageIdPrimary);

  //// Update buttons
  UpdateButtons();
}

void WebDialogUploadFiles::UpdateButtons()
{
  //// State
  int inputCount = 0;
  QPointer<QLabel>    errorLabel;
  QPointer<QWidget>   bodyWidget;
  QPointer<DnDFiles>  fileWidget;

  //// Buttons
  switch (GetPage())
  {
  case PageIdPrimary:
    {
      inputCount = PagePrimaryFiles->GetFilesCount();
      errorLabel = PagePrimaryError;
      bodyWidget = PagePrimaryBody;
      fileWidget = PagePrimaryFiles;
    }
    break;
  case PageIdSecondary:
    {
      inputCount = PageSecondaryFiles->GetFilesCount();
      errorLabel = PageSecondaryError;
      bodyWidget = PageSecondaryBody;
      fileWidget = PageSecondaryFiles;
    }
    break;
  default:
    break;
  }

  //// Error text
  QString errorMessage;
  if (inputCount > Details::MaximumFiles)
  {
    errorMessage = tr("Maximum number of files exceeded");
  }
  else if (fileWidget->HasErrors())
  {
    errorMessage = tr("A chromatogram cannot be used as primary and auxiliary simultaniously");
  }

  //// Can continue?
  bool isEmpty     = inputCount == 0;
  bool canContinue = !isEmpty && errorMessage.isEmpty();

  //// Apply / error label
  errorLabel->setText(errorMessage);
  errorLabel->setVisible(!errorMessage.isEmpty());

  //// Apply / frame
  if (bodyWidget->property("error").toBool() != !errorMessage.isEmpty())
  {
    bodyWidget->setProperty("error", !errorMessage.isEmpty());
    bodyWidget->style()->unpolish(bodyWidget);
    bodyWidget->style()->polish(bodyWidget);
  }

  //// Apply / buttons
  ButtonBox->button(QDialogButtonBox::Apply)->setEnabled(canContinue);
}

void WebDialogUploadFiles::Accept()
{
//  if (GetPage() == PageIdPrimary)
//  {
//    //// Apply mutually excluded files
//    PageSecondaryFiles->SetUnacceptableFiles(PagePrimaryFiles->GetFiles());

//    //// Proceed
//    SetPage(PageIdSecondary);
//  }
//  else
  if(true)
  {
    //// Get ids
    if (auto contextRoot = GenesisContextRoot::Get())
    {
      if (auto contextProject = contextRoot->GetContextProject())
      {
        int projectId = contextProject->GetData(Names::ContextTagNames::ProjectId).toInt();
        if (projectId)
        {
          //// Upload handlers
          auto handle = [/*can't capture this, shall close, access through statics*/](QNetworkReply* r, QJsonDocument /*reply*/)
          {
                        //// Reset file models
                        TreeModel::ResetInstances("TreeModelDynamicProjectImportedFiles");

                        //// Notify upload group
                        Notification::NotifyMulti("files/cdf/upload", tr("Uploading files"));

                        //// Notify upload item
                        Notification::NotifySubMultiSuccess(QString("files/cdf/upload/%1").arg(r->property("tag").toString()),
                                                            tr("File \"%1\" successfully imported").arg(r->property("tag").toString()));
        };
          auto error = [/*can't capture this, shall close, access through statics*/](QNetworkReply* r, QNetworkReply::NetworkError error)
          {
                       //// Notify upload group
                       Notification::NotifyMulti("files/cdf/upload", tr("Uploading files"));

                       //// Notify upload item
                       Notification::NotifySubMultiError(QString("files/cdf/upload/%1").arg(r->property("tag").toString()),
                                                         tr("Failed to import file \"%1\"").arg(r->property("tag").toString()),
                                                         error);
        };

          //// Upload
          {
            //// Clear upload list
            Notification::NotifyMultiClear("files/cdf/upload");
            Notification::NotifyMulti("files/cdf/upload", tr("Uploading files"));

            //// Primaries
            auto& filesPrimary = PagePrimaryFiles->GetFilesData();
            for (auto f = filesPrimary.begin(); f != filesPrimary.end(); f++)
            {
              QString tag = f.key().split("/").last();

              Notification::NotifySubMultiProgress(QString("files/cdf/upload/%1").arg(tag),
                                                   tr("Importing file \"%1\"").arg(tag));

              API::REST::UploadFile(f.key(), projectId, TreeModelDynamicFileTypes::ConstantFileTypeBase, f.value(), 0, "",
                                    handle,
                                    error
                                    );
            }
          }
        }
      }
    }

    //// Open files page
    GenesisWindow::Get()->ShowPage(ViewProjectImportedFilesPageName);
//    GenesisContextRoot::Get()->GetContextUi()->SetData(ContextTagNames::Page, ViewProjectImportedFilesPageName);

    //// Done
    WebDialog::Accept();
  }
}

void WebDialogUploadFiles::Reject()
{
  //// Handle cancel
  if (GetPage() == PageIdSecondary)
  {
    SetPage(PageIdPrimary);
  }
  else
  {
    WebDialog::Reject();
  }
}

void WebDialogUploadFiles::SetPage(PageId pageId)
{
  switch (pageId)
  {
    case PageIdPrimary:
      {
        //// Page
        Pages->setCurrentWidget(PagePrimary);

        //// Buttons
        if (auto cancel = ButtonBox->button(QDialogButtonBox::Cancel))
        {
          cancel->setText(" " + tr("Cancel"));
          cancel->setIcon(QIcon());
        }
        if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
        {
            apply->setText(tr("Import files") + " ");
            apply->setIcon(QIcon(":/resource/icons/icon_upload_light.png"));

//          apply->setText(tr("Next step") + " ");
//          apply->setIcon(QIcon(":/resource/icons/icon_wizard_next_light.png"));
//          apply->setLayoutDirection(Qt::RightToLeft);
        }
      }
      break;
    case PageIdSecondary:
      {
        //// Page
        Pages->setCurrentWidget(PageSecondary);

        //// Buttons
        if (auto cancel = ButtonBox->button(QDialogButtonBox::Cancel))
        {
          cancel->setText(" " + tr("Previous step"));
          cancel->setIcon(QIcon(":/resource/icons/icon_wizard_prev.png"));
        }
        if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
        {
          apply->setText(tr("Import files") + " ");
          apply->setIcon(QIcon(":/resource/icons/icon_upload_light.png"));
        }
      }
      break;
    default:
      break;
  }
  UpdateButtons();
}

WebDialogUploadFiles::PageId WebDialogUploadFiles::GetPage()
{
  if (Pages->currentWidget() == PagePrimary)
  {
    return PageIdPrimary;
  }
  if (Pages->currentWidget() == PageSecondary)
  {
    return PageIdSecondary;
  }
  return PageIdLast;
}
