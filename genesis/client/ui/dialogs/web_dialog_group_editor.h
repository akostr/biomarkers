#pragma once
#ifndef WEB_DIALOG_IMPORT_TABLE_GROUP_EDITOR
#define WEB_DIALOG_IMPORT_TABLE_GROUP_EDITOR

#include "ui/dialogs/templates/dialog.h"
#include "ui/widgets/edit_group_widget.h"

#include <QComboBox>
#include <QRadioButton>

class QLabel;

namespace Dialogs
{
  class WebDialogGroupEditor final : public Templates::Dialog
  {
      Q_OBJECT
  public:
    explicit WebDialogGroupEditor(QWidget* parent = nullptr);
    explicit WebDialogGroupEditor(const QString& dialogHeader, QWidget* parent = nullptr);
    ~WebDialogGroupEditor() = default;

    void SetCaptionLabel(const QString& captionLabel);
    void SetUngroupRadioButtonText(const QString& ungroupText);
    void SetGroupRadioButtonText(const QString& groupText);

    void SetGroupInfo(const std::map<QString, int>& groupInfo);
    QPair<int, QString> GetGroupInfo() const;
    bool IsAddToGroup();

  protected:
    void SetupUi();
    void DisableOkButton();

    QPointer<Widgets::EditGroupWidget> EditGroup;
    QString textBeforeRemove;
  };
}
#endif
