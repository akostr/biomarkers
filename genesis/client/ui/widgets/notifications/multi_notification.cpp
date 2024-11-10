#include "multi_notification.h"
#include "ui_multi_notification.h"



MultiNotification::MultiNotification(Notification *notification, QWidget *parent) :
  NotificationWidget(notification, parent),
  ui(new Ui::MultiNotification),
  uploadSuccessIcon(new QIcon(":/resource/icons/icon_upload_done.png")),
  uploadProgressIcon(new QIcon(":/resource/icons/icon_upload_progress.png")),
  uploadErrorIcon(new QIcon(":/resource/styles/standard/icons/icon_error.png"))
{
  setupUi();
}

MultiNotification::~MultiNotification()
{
  delete ui;
}

void MultiNotification::setupUi()
{
  mContent->setObjectName("MultiNotificationContent");
  ui->setupUi(mContent);
  ui->textLabel->setProperty("style", "header");
  connect(ui->closeBtn, &QPushButton::clicked, this, &NotificationWidget::onNotificationClosed);
  connect(ui->collapseBtn, &QPushButton::clicked, this, &MultiNotification::onCollapseBtnClicked);
  mMultiLayout = new QVBoxLayout();
  ui->multiContent->setLayout(mMultiLayout);
  mMultiLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Expanding));
  updateData();
}

void MultiNotification::updateData()
{
  if(!mNotification)
    return;
  ui->textLabel->setText(mNotification->GetText());
  auto children = mNotification->GetChildrenRecursiveOrdered();
  auto removeList = mIdToChildMap.keys();

  bool isFinished = true;
  for(auto& c : children)
  {
    handleChildData(c);
    removeList.removeAll(c->GetNodeId());
    if(c->GetStatus() != Notification::StatusSuccess &&
       c->GetStatus() != Notification::StatusError)
      isFinished = false;
  }
  if(!removeList.isEmpty())
  {
    for(auto& key : removeList)
    {
      delete mIdToChildMap[key];
      mIdToChildMap.remove(key);
    }
  }
  if(isFinished && ui->closeBtn->isHidden())
    ui->closeBtn->show();
  else if(!isFinished && !ui->closeBtn->isHidden())
    ui->closeBtn->hide();
}

void MultiNotification::collapse()
{
  ui->scrollArea->hide();
}

void MultiNotification::expand()
{
  ui->scrollArea->show();
}

void MultiNotification::onCollapseBtnClicked(bool checked)
{
  if(checked)
    expand();
  else
    collapse();
}

void MultiNotification::onNotificationUpdate()
{
  updateData();
}

void MultiNotification::handleChildData(Notification* child)
{
  auto id = child->GetNodeId();
  MultiChildWidget* row = nullptr;
  if(!mIdToChildMap.contains(id))
  {
    auto newRow = new MultiChildWidget(ui->multiContent);
    mMultiLayout->insertWidget(mMultiLayout->count() - 1, newRow);
    mIdToChildMap[id] = newRow;
    row = newRow;
  }
  else
  {
    row = mIdToChildMap[id];
  }
  row->setText(child->GetText());
  switch(child->GetStatus())
  {
  case Notification::StatusProgress:
    row->setIcon(*uploadProgressIcon);
    break;
  case Notification::StatusSuccess:
    row->setIcon(*uploadSuccessIcon);
    break;
  case Notification::StatusError:
    row->setIcon(*uploadErrorIcon);
    break;
  default:
    row->resetIcon();
    break;
  }
}

MultiChildWidget::MultiChildWidget(QWidget *parent)
  : QWidget(parent)
{
  auto l = new QHBoxLayout();
  l->setContentsMargins(0,0,0,0);
  l->setSpacing(0);
  setLayout(l);
  mTextLabel = new QLabel();
  mIconLabel = new QLabel();
  l->addWidget(mTextLabel,1);
  l->addWidget(mIconLabel, 0);
}

void MultiChildWidget::setText(const QString &text)
{
  mTextLabel->setText(text);
}

void MultiChildWidget::setIcon(const QIcon &icon)
{
  mIconLabel->setPixmap(icon.pixmap({64, 64}, devicePixelRatio()));
}

void MultiChildWidget::resetIcon()
{
  mIconLabel->setText("EI");
}
