#include "web_dialog_group_editor.h"

#include "api/api_rest.h"
#include "logic/notification.h"
#include "genesis_style/style.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>

using namespace Widgets;

namespace Dialogs
{
  WebDialogGroupEditor::WebDialogGroupEditor(QWidget* parent)
    : Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  {
    Size = QSizeF(0, 0);
    SetupUi();
    auto s = Templates::Dialog::Settings();
    s.dialogHeader = tr("Change groups");
    applySettings(s);
  }

  WebDialogGroupEditor::WebDialogGroupEditor(const QString& dialogHeader, QWidget* parent)
    : Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  {
    Size = QSizeF(0, 0);
    SetupUi();
    auto s = Templates::Dialog::Settings();
    s.dialogHeader = dialogHeader;
    applySettings(s);
  }

  void WebDialogGroupEditor::SetCaptionLabel(const QString& captionLabel)
  {
    EditGroup->SetCaptionText(captionLabel);
  }

  void WebDialogGroupEditor::SetUngroupRadioButtonText(const QString& ungroupText)
  {
    EditGroup->SetUngroupRadioText(ungroupText);
  }

  void WebDialogGroupEditor::SetGroupRadioButtonText(const QString& groupText)
  {
    EditGroup->SetGroupRadioText(groupText);
  }

  void WebDialogGroupEditor::SetGroupInfo(const std::map<QString, int>& groupInfo)
  {
    EditGroup->SetGroupInfo(groupInfo);
  }

  QPair<int, QString> WebDialogGroupEditor::GetGroupInfo() const
  {
    return EditGroup->GetGroupInfo();
  }

  bool WebDialogGroupEditor::IsAddToGroup()
  {
    return EditGroup->IsGroupChecked();
  }

  void WebDialogGroupEditor::SetupUi()
  {
    auto content = getContent();
    auto body = new QWidget(content);
    content->layout()->addWidget(body);

    auto bodyLayout = new QVBoxLayout();
    body->setLayout(bodyLayout);

    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    EditGroup = new EditGroupWidget(this);
    bodyLayout->addWidget(EditGroup);
    bodyLayout->addStretch(1);
    connect(EditGroup, &EditGroupWidget::ungroupRadioClicked, this, &WebDialogGroupEditor::DisableOkButton);
    connect(EditGroup, &EditGroupWidget::groupComboTextChanged, this, &WebDialogGroupEditor::DisableOkButton);
    ButtonBox->button(QDialogButtonBox::Ok)->setText(tr("Move"));
    Size = QSizeF(0.3, 0.4);
  }

  void WebDialogGroupEditor::DisableOkButton()
  {
    ButtonBox->button(QDialogButtonBox::Ok)->setDisabled(EditGroup->IsGroupChecked() && EditGroup->GroupTitle().isEmpty());
  }
}
