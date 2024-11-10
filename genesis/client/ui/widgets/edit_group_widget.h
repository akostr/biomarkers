#pragma once
#ifndef EDIT_GROUP_WIDGET_H
#define EDIT_GROUP_WIDGET_H

#include <QWidget>

namespace Ui
{
  class EditGroupWidget;
}

namespace Widgets
{
  class EditGroupWidget : public QWidget
  {
    Q_OBJECT
  public:
    explicit EditGroupWidget(QWidget* parent = nullptr);
    ~EditGroupWidget();

    bool IsCommonChecked() const;
    bool IsGroupChecked() const;
    bool IsCreateNewGroup() const;
    int SelectedGroupId() const;
    QString GroupTitle() const;

    void SetCaptionText(const QString& text);
    void SetUngroupRadioText(const QString& text);
    void SetGroupRadioText(const QString& text);
    void SetGroupInfo(const std::map<QString, int>& groupInfo);
    QPair<int, QString> GetGroupInfo() const;
    void SetSelectedGroupId(int);

  signals:
    void groupComboTextChanged(const QString& text);
    void ungroupRadioClicked(bool toggled);

  private:
    Ui::EditGroupWidget* ui = nullptr;

    void SetupUi();
    void ConnectSignals();
  };
}
#endif