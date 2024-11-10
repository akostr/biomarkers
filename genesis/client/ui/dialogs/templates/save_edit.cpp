#include "save_edit.h"
#include <QLabel>
#include <QStyle>
#include "ui_save_edit.h"
#include <genesis_style/style.h>
#include <logic/notification.h>

namespace Dialogs
{
  namespace Templates
  {
    enum NameState
    {
      state_none = 0,
      state_unchecked,
      state_valid,
      state_invalid
    };

    SaveEdit::SaveEdit(QWidget* parent)
      : WebDialog(parent, nullptr, QSize(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
      ui(new Ui::SaveEdit),
      mPMValid("://resource/styles/standard/qmenu/checkbox_context_on@2x.png"),
      mPMInvalid("://resource/styles/standard/icons/input_error_warning@2x.png"),
      mPMUnchecked("://resource/styles/standard/qmenu/checkbox_context_off@2x.png"),
      mContent(new QWidget(Content))
    {
      ui->setupUi(mContent);
      setupUi();
      applySettings(Settings());
      Content->layout()->addWidget(mContent);
      Size = QSizeF(0.3, 0.4);
      UpdateGeometry();
    }

    SaveEdit::SaveEdit(QWidget* parent, const Settings& settings)
      : WebDialog(parent, nullptr, QSize(0.2, 0.1), QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
      ui(new Ui::SaveEdit),
      mPMValid("://resource/styles/standard/qmenu/checkbox_context_on@2x.png"),
      mPMInvalid("://resource/styles/standard/icons/input_error_warning@2x.png"),
      mPMUnchecked("://resource/styles/standard/qmenu/checkbox_context_off@2x.png"),
      mContent(new QWidget(this))
    {
      ui->setupUi(mContent);
      setupUi();
      applySettings(settings);
      Content->layout()->addWidget(mContent);
      Size = QSizeF(0.3, 0.4);
      UpdateGeometry();
    }

    SaveEdit::~SaveEdit()
    {
      delete ui;
    }

    void SaveEdit::applySettings(const Settings& newSettings)
    {
      mSettings = newSettings;
      ButtonBox->button(QDialogButtonBox::Ok)->setText(mSettings.okBtnText);
      ButtonBox->button(QDialogButtonBox::Cancel)->setText(mSettings.cancelBtnText);

      ui->captionLabel->setText(mSettings.header);
      ui->subCaptionLabel->setText(mSettings.subHeader);
      if (mSettings.subHeader.isEmpty())
      {
        if (!ui->subCaptionLabel->isHidden())
          ui->subCaptionLabel->hide();
      }
      else
      {
        if (ui->subCaptionLabel->isHidden())
          ui->subCaptionLabel->show();
      }
      ui->titleCommentWidget->SetTitle(newSettings.defaultTitle);
      ui->titleCommentWidget->SetComment(newSettings.defaultComment);
      ui->titleCommentWidget->SetForbiddenTitles(newSettings.forbiddenTitles);
      setDialogDataValidity(ui->titleCommentWidget->IsValid());
    }

    SaveEdit::Settings SaveEdit::settings()
    {
      return mSettings;
    }

    QString SaveEdit::getTitle()
    {
      return ui->titleCommentWidget->Title();
    }

    QString SaveEdit::getComment()
    {
      return ui->titleCommentWidget->Comment();
    }

    void SaveEdit::setupUi()
    {
      ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());
      ui->subCaptionLabel->hide();

      connect(ui->closeButton, &QPushButton::clicked, this, &WebDialog::Reject);
      connect(ui->titleCommentWidget, &Widgets::TitleCommentWidget::titleTextChanged, this, [&]()
        {
          setDialogDataValidity(ui->titleCommentWidget->IsValid());
        });
    }

    void SaveEdit::setDialogDataValidity(bool isValid)
    {
      auto btn = ButtonBox->button(QDialogButtonBox::Ok);
      if (isValid && !btn->isEnabled())
      {
        ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
      }
      else if (!isValid && btn->isEnabled())
      {
        ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      }
    }

    void SaveEdit::Accept()
    {
      if ((mSettings.commentRequired && ui->titleCommentWidget->IsValid()) || !mSettings.commentRequired)
        //if data is valid
      {
        WebDialog::Accept();
      }
      else
      {
        Notification::NotifyError(tr("Data is not valid"));
        return;
      }
    }
  }
}
