#include "edit_group_widget.h"

#include "ui_edit_group_widget.h"
#include "logic/known_context_tag_names.h"
#include "logic/known_json_tag_names.h"
#include "api/api_rest.h"
#include "logic/notification.h"
#include "logic/context_root.h"
#include "genesis_style/style.h"

#include <QLineEdit>

using namespace Core;

namespace Widgets
{
  EditGroupWidget::EditGroupWidget(QWidget* parent)
    :QWidget(parent), ui(new Ui::EditGroupWidget())
  {
    SetupUi();
    ConnectSignals();
  }

  EditGroupWidget::~EditGroupWidget()
  {
    delete ui;
  }

  bool EditGroupWidget::IsCommonChecked() const
  {
    return ui->commonRadioButton->isChecked();
  }

  bool EditGroupWidget::IsGroupChecked() const
  {
    return ui->groupRadioButton->isChecked();
  }

  bool EditGroupWidget::IsCreateNewGroup() const
  {
    return ui->groupRadioButton->isChecked() && ui->groupComboBox->currentIndex() == -1;
  }

  int EditGroupWidget::SelectedGroupId() const
  {
    const auto index = ui->groupComboBox->currentIndex();
    const auto id = ui->groupComboBox->itemData(index);
    return id.toInt();
  }

  QString EditGroupWidget::GroupTitle() const
  {
    return ui->groupComboBox->currentText();
  }

  void EditGroupWidget::SetCaptionText(const QString& text)
  {
    ui->label->setText(text);
  }

  void EditGroupWidget::SetUngroupRadioText(const QString& text)
  {
    ui->commonRadioButton->setText(text);
  }

  void EditGroupWidget::SetGroupRadioText(const QString& text)
  {
    ui->groupRadioButton->setText(text);
  }

  void EditGroupWidget::SetGroupInfo(const std::map<QString, int>& groupInfo)
  {
    ui->groupComboBox->clear();
    for (const auto& [name, id] : groupInfo)
      ui->groupComboBox->insertItem(id, name, id);
    ui->groupComboBox->setCurrentIndex(-1);
  }

  QPair<int, QString> EditGroupWidget::GetGroupInfo() const
  {
    const auto currentDataString = ui->groupComboBox->currentData(Qt::EditRole).toString();
    if (currentDataString != ui->groupComboBox->currentText())
      return { 0, ui->groupComboBox->currentText() };
    const auto data = ui->groupComboBox->currentData();
    return data.isValid() && !data.isNull()
      ? qMakePair(data.toInt(), QString())
      : qMakePair(0, ui->groupComboBox->currentText());
  }

  void EditGroupWidget::SetupUi()
  {
    ui->setupUi(this);
    ui->groupRadioButton->setChecked(false);
    ui->commonRadioButton->setChecked(true);
    ui->groupComboBox->setVisible(false);
    ui->groupComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    ui->groupComboBox->lineEdit()->setPlaceholderText(tr("Enter new group name or select from the list"));
    QPalette pal = ui->groupComboBox->lineEdit()->palette();
    pal.setColor(QPalette::ColorRole::PlaceholderText, Qt::lightGray);
    ui->groupComboBox->lineEdit()->setPalette(pal);
    ui->label->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  }

  void EditGroupWidget::ConnectSignals()
  {
    connect(ui->groupRadioButton, &QRadioButton::toggled, this, [&](bool checked) { ui->groupComboBox->setVisible(checked); });
    connect(ui->groupComboBox, &QComboBox::currentTextChanged, this, &EditGroupWidget::groupComboTextChanged);
    connect(ui->commonRadioButton, &QRadioButton::toggled, this, &EditGroupWidget::ungroupRadioClicked);
  }

  void EditGroupWidget::SetSelectedGroupId(int index)
  {
    ui->groupRadioButton->setChecked(true);
    ui->groupComboBox->setCurrentIndex(index);

    int id = ui->groupComboBox->currentIndex();
    QString title = ui->groupComboBox->currentText();
    int data = ui->groupComboBox->currentData().toInt();
    ui->groupComboBox->setCurrentIndex(index);

    int size = ui->groupComboBox->model()->rowCount();
  }
}


