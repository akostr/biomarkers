#include "module_plate.h"
#include "ui_module_plate.h"
#include "genesis_style/style.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QStyleOption>

#include <QStyle>
#include <QPainter>
#include <QMouseEvent>

namespace
{
QColor richer(const QColor &original, qreal factor);
}

ModulePlate::ModulePlate(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ModulePlate),
  mAvailable(false),
  mAnimationDurationMs(500),
  mInactiveHeadColor(QRgb(0xf6f8fa))
{
  mCurrentHeadColor = mInactiveHeadColor;
  ui->setupUi(this);
  setupUi();
}

ModulePlate::~ModulePlate()
{
  delete ui;
}

QColor ModulePlate::HeadColor() const
{
  return mHeadColor;
}

void ModulePlate::setHeadColor(const QColor &newHeadColor)
{
  if (mHeadColor == newHeadColor)
    return;
  mHeadColor = newHeadColor;
  emit HeadColorChanged();
}

QPixmap ModulePlate::Pixmap() const
{
  return mPixmap;
}

void ModulePlate::setPixmap(const QPixmap &newPixmap)
{
  mPixmap = newPixmap;
  emit PixmapChanged();
}

QString ModulePlate::Title() const
{
  return mTitle;
}

void ModulePlate::setTitle(const QString &newTitle)
{
  if (mTitle == newTitle)
    return;
  mTitle = newTitle;
  emit TitleChanged();
}

bool ModulePlate::Available() const
{
  return mAvailable;
}

void ModulePlate::setAvailable(bool newAvailable)
{
  if (mAvailable == newAvailable)
    return;
  mAvailable = newAvailable;
  emit AvailableChanged();
}

QString ModulePlate::Details() const
{
  return mDetails;
}

void ModulePlate::setDetails(const QString &newDetails)
{
  if (mDetails == newDetails)
    return;
  mDetails = newDetails;
  emit DetailsChanged();
}

int ModulePlate::AnimationDuration() const
{
  return mAnimationDurationMs;
}

void ModulePlate::setAnimationDuration(int newAnimationDuration)
{
  if (mAnimationDurationMs == newAnimationDuration)
    return;
  mAnimationDurationMs = newAnimationDuration;
  emit AnimationDurationChanged();
}

QPixmap ModulePlate::ActivePixmap() const
{
  return mActivePixmap;
}

void ModulePlate::setActivePixmap(const QPixmap &newActivePixmap)
{
  mActivePixmap = newActivePixmap;
  emit ActivePixmapChanged();
}

QPixmap ModulePlate::InactivePixmap() const
{
  return mInactivePixmap;
}

void ModulePlate::setInactivePixmap(const QPixmap &newInactivePixmap)
{
  mInactivePixmap = newInactivePixmap;
  emit InactivePixmapChanged();
}

QColor ModulePlate::CurrentHeadColor() const
{
  return mCurrentHeadColor;
}

void ModulePlate::setCurrentHeadColor(const QColor &newCurrentHeadColor)
{
  if (mCurrentHeadColor == newCurrentHeadColor)
    return;
  mCurrentHeadColor = newCurrentHeadColor;
  emit CurrentHeadColorChanged();
}

void ModulePlate::setupUi()
{
  setStyleSheet("QWidget#ModulePlate {border: 1px solid rgb(226, 231, 235); border-radius: 8px;}");
  style()->polish(this);
  ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());
  ui->stateLabel->setStyleSheet(Style::Genesis::Fonts::InterFont());
  ui->detailsLabel->setStyleSheet(Style::ApplySASS("QLabel {font: @RegularTextFont; color: @textColorPale;}"));
  ui->detailsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui->detailsLabel->setOpenExternalLinks(true);
  connect(this, &ModulePlate::AvailableChanged, this, [this]()
          {
            ui->stateLabel->setText(mAvailable ? tr("INSTALLED").toUpper() : tr("NOT PURCHASED").toUpper());
            ui->stateLabel->setProperty("style", mAvailable ? "available" : "unavailable");
            ui->stateLabel->style()->polish(ui->stateLabel);
            if(mAvailable)
              setCurrentHeadColor(mHeadColor);
            else
              setCurrentHeadColor(mInactiveHeadColor);
          });
  connect(this, &ModulePlate::PixmapChanged, this, [this]()
          {
            ui->iconLabel->setPixmap(mPixmap);
          });
  connect(this, &ModulePlate::InactivePixmapChanged, this, [this]()
          {
            if(mHoverInactiveAnimation)
              for(int i = 0; i <  mHoverInactiveAnimation->animationCount(); i++)
                if(auto anim = qobject_cast<QPropertyAnimation*>(mHoverInactiveAnimation->animationAt(i)))
                  if(anim->propertyName() == "Pixmap")
                    anim->setStartValue(mInactivePixmap);
          });
  connect(this, &ModulePlate::ActivePixmapChanged, this, [this]()
          {
            if(mHoverInactiveAnimation)
              for(int i = 0; i <  mHoverInactiveAnimation->animationCount(); i++)
                if(auto anim = qobject_cast<QPropertyAnimation*>(mHoverInactiveAnimation->animationAt(i)))
                  if(anim->propertyName() == "Pixmap")
                    anim->setEndValue(mActivePixmap);
          });
  connect(this, &ModulePlate::TitleChanged, this, [this]()
          {
            ui->captionLabel->setText(mTitle);
          });
  connect(this, &ModulePlate::DetailsChanged, this, [this]()
          {
            ui->detailsLabel->setText(mDetails);
          });
  connect(this, &ModulePlate::AnimationDurationChanged, this, [this]()
          {
            if(mHoverInactiveAnimation)
              for(int i = 0; i <  mHoverInactiveAnimation->animationCount(); i++)
                if(auto anim = qobject_cast<QVariantAnimation*>(mHoverInactiveAnimation->animationAt(i)))
                  anim->setDuration(mAnimationDurationMs);
            if(mHoverActiveAnimation)
              for(int i = 0; i <  mHoverActiveAnimation->animationCount(); i++)
                if(auto anim = qobject_cast<QVariantAnimation*>(mHoverActiveAnimation->animationAt(i)))
                  anim->setDuration(mAnimationDurationMs);
          });

  emit AvailableChanged();
  emit PixmapChanged();
  emit TitleChanged();
  emit DetailsChanged();

  mHoverInactiveAnimation = new QParallelAnimationGroup(this);

  auto headColorInactiveAnimation = new QPropertyAnimation(mHoverInactiveAnimation);
  headColorInactiveAnimation->setTargetObject(this);
  headColorInactiveAnimation->setPropertyName("CurrentHeadColor");
  headColorInactiveAnimation->setStartValue(mInactiveHeadColor);
  headColorInactiveAnimation->setEndValue(mHeadColor);
  headColorInactiveAnimation->setDuration(mAnimationDurationMs);

  auto pixmapTransitionAnimation = new QPropertyAnimation(mHoverInactiveAnimation);
  pixmapTransitionAnimation->setTargetObject(this);
  pixmapTransitionAnimation->setPropertyName("Pixmap");
  pixmapTransitionAnimation->setStartValue(mInactivePixmap);
  pixmapTransitionAnimation->setEndValue(mActivePixmap);
  pixmapTransitionAnimation->setDuration(mAnimationDurationMs);

  mHoverActiveAnimation = new QParallelAnimationGroup(this);

  auto headColorActiveAnimation = new QPropertyAnimation(mHoverActiveAnimation);
  headColorActiveAnimation->setTargetObject(this);
  headColorActiveAnimation->setPropertyName("CurrentHeadColor");
  headColorActiveAnimation->setStartValue(mHeadColor);
  headColorActiveAnimation->setEndValue(richer(mHeadColor, 2));
  headColorActiveAnimation->setDuration(mAnimationDurationMs);


  connect(this, &ModulePlate::HeadColorChanged, this, [this, headColorInactiveAnimation, headColorActiveAnimation]()
          {
            if(mAvailable)
              setCurrentHeadColor(mHeadColor);
            headColorInactiveAnimation->setEndValue(mHeadColor);
            headColorActiveAnimation->setStartValue(mHeadColor);
            headColorActiveAnimation->setEndValue(richer(mHeadColor, 2));
          });
  connect(this, &ModulePlate::CurrentHeadColorChanged, this, [this]()
          {
            update();
          });
}

void ModulePlate::paintEvent(QPaintEvent *event)
{
  QPainter p(this);

  QStyleOption opt;
  opt.initFrom(this);
  style()->drawPrimitive(QStyle::PE_Widget,&opt,&p, this);
  auto headRect = rect();
  headRect.setHeight(80);
  headRect.setWidth(headRect.width()-2);
  headRect.moveTopLeft({1,1});
  p.setBrush(mCurrentHeadColor);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(headRect, 8, 8);
  p.end();
  QWidget::paintEvent(event);
}

void ModulePlate::enterEvent(QEnterEvent *event)
{
  if(mAvailable)
  {
    if(mHoverInactiveAnimation->state() == QParallelAnimationGroup::Running)
      mHoverInactiveAnimation->stop();
    mHoverActiveAnimation->setDirection(QAbstractAnimation::Forward);
    if(mHoverActiveAnimation->state() != QParallelAnimationGroup::Running)
      mHoverActiveAnimation->start();
  }
  else
  {
    if(mHoverActiveAnimation->state() == QParallelAnimationGroup::Running)
      mHoverActiveAnimation->stop();
    mHoverInactiveAnimation->setDirection(QAbstractAnimation::Forward);
    if(mHoverInactiveAnimation->state() != QParallelAnimationGroup::Running)
      mHoverInactiveAnimation->start();
  }
}

void ModulePlate::leaveEvent(QEvent *event)
{
  if(mAvailable)
  {
    if(mHoverInactiveAnimation->state() == QParallelAnimationGroup::Running)
      mHoverInactiveAnimation->stop();
    mHoverActiveAnimation->setDirection(QAbstractAnimation::Backward);
    if(mHoverActiveAnimation->state() != QParallelAnimationGroup::Running)
      mHoverActiveAnimation->start();
  }
  else
  {
    if(mHoverActiveAnimation->state() == QParallelAnimationGroup::Running)
      mHoverActiveAnimation->stop();
    mHoverInactiveAnimation->setDirection(QAbstractAnimation::Backward);
    if(mHoverInactiveAnimation->state() != QParallelAnimationGroup::Running)
      mHoverInactiveAnimation->start();
  }
}

void ModulePlate::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
    emit clicked();
  QWidget::mousePressEvent(event);
}

namespace
{
QColor richer(const QColor& original, qreal factor)
{
  auto hsv = original.toHsv();
  hsv.setHsvF(hsv.hueF(), hsv.saturationF() * factor, hsv.valueF());
  return hsv;
}
}
