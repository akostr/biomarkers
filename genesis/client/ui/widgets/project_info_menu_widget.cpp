#include "project_info_menu_widget.h"
#include "ui_project_info_menu_widget.h"
#include "genesis_style/style.h"

#include <QPainter>
#include <QPalette>
#include <QStyleOptionFrame>

ProjectInfoMenuWidget::ProjectInfoMenuWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ProjectInfoMenuWidget)
{
  ui->setupUi(this);
  ui->projectNameLabel->setStyleSheet(Style::Genesis::Fonts::RegularBold());
  ui->projectNameLabel->setScaledContents(true);
  ui->projectNameLabel->setMinimumWidth(0);
  ui->projectNameLabel->setWordWrap(true);
  ui->projectGroupLabel->setStyleSheet(Style::Genesis::Fonts::SecondaryText());
  setStyleSheet(
"ProjectInfoMenuWidget {border-radius: 8px; margin-right: 5px;}"
"ProjectInfoMenuWidget:hover {background-color: rgb(245, 247, 249);}"
"ProjectInfoMenuWidget:checked {background-color: rgb(237, 242, 245);}"
    );
}

ProjectInfoMenuWidget::~ProjectInfoMenuWidget()
{
  delete ui;
}

void ProjectInfoMenuWidget::setTitle(const QString &title)
{
  ui->projectNameLabel->setText(title);
}

void ProjectInfoMenuWidget::setGroup(const QString &group)
{
  ui->projectGroupLabel->setText(group);
}

void ProjectInfoMenuWidget::setPixmap(const QPixmap &pixmap)
{
  ui->projectStatusLabel->setPixmap(pixmap);
}

bool ProjectInfoMenuWidget::isChecked() const
{
  return mIsChecked;
}

void ProjectInfoMenuWidget::setCheckState(bool isChecked)
{
  mIsChecked = isChecked;
  emit checkStateChanged(mIsChecked);
  update();
}


void ProjectInfoMenuWidget::mouseReleaseEvent(QMouseEvent *event)
{
  setCheckState(!isChecked());
}

void ProjectInfoMenuWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  QStyleOptionFrame option;
  option.initFrom(this);
  if(isChecked())
    option.state |= QStyle::State_On;
  style()->drawPrimitive(QStyle::PE_Widget, &option, &painter, this);
}
