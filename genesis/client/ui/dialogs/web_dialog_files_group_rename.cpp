#include "web_dialog_files_group_rename.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include <api/api_rest.h>
#include <genesis_style/style.h>
#include <logic/notification.h>

WebDialogFilesGroupRename::WebDialogFilesGroupRename(int groupId, const QString& groupName, QWidget *parent)
  : WebDialog(parent, nullptr, QSizeF(0, 0), QDialogButtonBox::Save | QDialogButtonBox::Cancel)
  , GroupId(groupId)
  , GroupName(groupName)
{
  SetupUi();
}

WebDialogFilesGroupRename::~WebDialogFilesGroupRename()
{

}

QString WebDialogFilesGroupRename::GetGroupName()
{
  return GroupNameLineEdit->text();
}

void WebDialogFilesGroupRename::SetupUi()
{
  auto body = new QWidget(Content);
  Content->layout()->addWidget(body);

  auto bodyLayout = new QVBoxLayout();
  body->setLayout(bodyLayout);

  bodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  bodyLayout->setSpacing(0);

  //// Content
  {
    //// Header
    {
      QLabel* caption = new QLabel(tr("Group name editing"), body);
      bodyLayout->addWidget(caption);

      caption->setAlignment(Qt::AlignCenter);
      caption->setStyleSheet(Style::Genesis::GetH1());
    }

    //// Space
    bodyLayout->addSpacing(Style::Scale(26));

    //// Input
    {
      //// Title
      {
        QLabel* caption = new QLabel(tr("Group name"), body);
        {
          QPalette pal = caption->palette();
          pal.setColor(QPalette::ColorRole::Text, Qt::lightGray);
          caption->setPalette(pal);
        }
        bodyLayout->addWidget(caption);
        bodyLayout->addSpacing(Style::Scale(4));

        GroupNameLineEdit = new QLineEdit(body);
        GroupNameLineEdit->setText(GroupName);
        bodyLayout->addWidget(GroupNameLineEdit);
        GroupNameLineEdit->setPlaceholderText(tr("Must be non empty"));
        {
          QPalette pal = GroupNameLineEdit->palette();
          pal.setColor(QPalette::ColorRole::PlaceholderText, Qt::lightGray);
          GroupNameLineEdit->setPalette(pal);
        }
        connect(GroupNameLineEdit, &QLineEdit::textEdited, this, [this](const QString& text)
        {
          if(text.isEmpty())
            ButtonBox->button(QDialogButtonBox::Save)->setDisabled(true);
          else
            ButtonBox->button(QDialogButtonBox::Save)->setDisabled(false);
        });
      }
    }
  }
}
