#include "collapsed_widgets_view.h"

#include <settings/settings.h>

#include <QVBoxLayout>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <QTimer>
#include <QTransform>
#include <QSplitter>
#include <QDebug>
#include <QStyle>
#include <QCoreApplication>

#ifndef DISABLE_CYBER_FRAC_STYLE
#include <style.h>
#endif

/////////////////////////////////////////////
//// Collapsed Widgets View Item
CollapsedWidgetsViewItem::CollapsedWidgetsViewItem(const QString& title, 
                                                   QWidget* parent, 
                                                   QLayout* layout, 
                                                   QWidget* target, 
                                                   const QString& path, 
                                                   bool hideInSplitter)
  : QPushButton(title, parent)
  , AssociatedWidget(target)
  , HideInSplitter(hideInSplitter)
  , Active(true)
  , Path(path)
{
  layout->addWidget(this);

  const int width = 35;
  setMinimumWidth(width);
  setMaximumWidth(width);
  setFlat(true);
  setFocusPolicy(Qt::NoFocus);

  Load();

  if (target)
  {
    target->installEventFilter(this);
  }

  connect(this, &QAbstractButton::clicked, [this](bool) 
  { 
    // May be collapsible
    SetAssociatedWidgetVisible(!IsAssociatedWidgetVisible());
  });

  if (AssociatedWidget)
  {
    QSplitter* parentSplitter = qobject_cast<QSplitter*>(AssociatedWidget->parentWidget());
    if (parentSplitter)
      connect(parentSplitter, &QSplitter::splitterMoved, this, &CollapsedWidgetsViewItem::updateWidget);
  }

  updateWidget();
}

bool CollapsedWidgetsViewItem::IsAssociatedWidgetVisible()
{
  if (!AssociatedWidget)
    return false;

  // May be collapsible
  int indexOfAssiciatedWidget = -1;
  int indexOfOppositeWidget = -1;
  QList<int> sizes;
  QSplitter* parentSplitter = GetParentSplitterForAssociatedWidget(&indexOfAssiciatedWidget, &indexOfOppositeWidget, &sizes);
  
  // Get state
  if (parentSplitter)
  {
    if (HideInSplitter)
    {
      return AssociatedWidget->isVisible()
          && (sizes[indexOfAssiciatedWidget] > 0 || (sizes.first() + sizes.last() == 0));
    }
    return sizes[indexOfAssiciatedWidget] > 0 || (sizes.first() + sizes.last() == 0);
  }
  return AssociatedWidget->isVisible();
}

void CollapsedWidgetsViewItem::SetAssociatedWidgetVisible(bool visibleArg)
{
  if (!AssociatedWidget)
    return;

  // May be collapsible
  int indexOfAssiciatedWidget = -1;
  int indexOfOppositeWidget = -1;
  QList<int> sizes;
  QSplitter* parentSplitter = GetParentSplitterForAssociatedWidget(&indexOfAssiciatedWidget, &indexOfOppositeWidget, &sizes);

  // Collapse / reveal
  if (parentSplitter)
  {
    bool visible = IsAssociatedWidgetVisible();
    if (visible != visibleArg)
    {
      // Restore splitter
      {
        if (indexOfAssiciatedWidget < sizes.size()
         && indexOfOppositeWidget < sizes.size())
        {
          if (visible)
          {
            sizes[indexOfOppositeWidget] += sizes[indexOfAssiciatedWidget];
            sizes[indexOfAssiciatedWidget] = 0;
          }
          else
          {
            int hint = (parentSplitter->orientation() == Qt::Vertical) ? AssociatedWidget->minimumHeight() : AssociatedWidget->minimumWidth();
            if (hint == 0)
              hint = (parentSplitter->orientation() == Qt::Vertical) ? AssociatedWidget->sizeHint().height() : AssociatedWidget->sizeHint().width();

            sizes[indexOfAssiciatedWidget] = hint;
            sizes[indexOfOppositeWidget] -= hint;
          }
          parentSplitter->setSizes(sizes);
        }
      }

      // Restore visibility
      if (HideInSplitter)
      {
        AssociatedWidget->setVisible(visibleArg);
        if (visibleArg && AssociatedWidget->parent())
          AssociatedWidget->parentWidget()->setVisible(true);
      }
    }
  }
  else
  {
    AssociatedWidget->setVisible(visibleArg);
    if (visibleArg && AssociatedWidget->parent())
      AssociatedWidget->parentWidget()->setVisible(true);
  }

  setProperty("targetvisible", visibleArg);
  style()->unpolish(this);
  style()->polish(this);

  Save(&visibleArg);
}

CollapsedWidgetsViewItem::~CollapsedWidgetsViewItem()
{
}

QWidget *CollapsedWidgetsViewItem::GetAssociatedWidget()
{
  return AssociatedWidget;
}

QSplitter* CollapsedWidgetsViewItem::GetParentSplitterForAssociatedWidget(int* indexOfAssiciatedWidgetArg, int* indexOfOppositeWidgetArg, QList<int>* sizesArg)
{
  if (!AssociatedWidget)
    return nullptr;

  QSplitter* parentSplitter = qobject_cast<QSplitter*>(AssociatedWidget->parentWidget());

  QList<int> sizes;
  if (parentSplitter)
  {
    sizes = parentSplitter->sizes();
  }

  // Get indexes
  int indexOfAssiciatedWidget = -1;
  int indexOfOppositeWidget = -1;
  if (parentSplitter)
  {
    indexOfAssiciatedWidget = parentSplitter->indexOf(AssociatedWidget);
    if (indexOfAssiciatedWidget == 0)
    {
      indexOfOppositeWidget = 1;
    }
    else
    {
      indexOfOppositeWidget = 0;
    }
  }
  
  if (indexOfAssiciatedWidgetArg)
    *indexOfAssiciatedWidgetArg = indexOfAssiciatedWidget;

  if (indexOfOppositeWidgetArg)
    *indexOfOppositeWidgetArg = indexOfOppositeWidget;

  if (sizesArg)
    *sizesArg = sizes;

  return parentSplitter;
}

QString CollapsedWidgetsViewItem::GetSettingsPath()
{
  QSplitter* splitter = GetParentSplitterForAssociatedWidget();
  if (splitter)
  {
    QString splitterPath = splitter->property("Settings").toString();
    if (!splitterPath.isEmpty())
      return splitterPath;

    splitterPath = Path;
    splitterPath += "/splitter";
    splitter->setProperty("Settings", splitterPath);
    return splitterPath;
  }
  return Path;
}

void CollapsedWidgetsViewItem::Load()
{
  if (!Active)
    return;

  if (!AssociatedWidget)
    return;

  QSplitter* splitter = GetParentSplitterForAssociatedWidget();
  if (splitter)
  {
    QString path = GetSettingsPath();
    std::string stored = Common::Settings::Get().GetValue(path).toString().toStdString();
    if (!stored.empty())
    {
      QByteArray splitterState64 = QByteArray::fromStdString(stored);
      QByteArray splitterState = QByteArray::fromBase64(splitterState64);
      splitter->restoreState(splitterState);
    }
  }
  else 
  {
    bool visible = Common::Settings::Get().GetValue(Path, true).toBool();

    QPointer<CollapsedWidgetsViewItem> instance = this;
    QTimer::singleShot(0, [instance, visible] {
      if (instance)
      {
        instance->SetAssociatedWidgetVisible(visible);
      }
    });
  }
  
  if (splitter && HideInSplitter)
  {
    bool visible = Common::Settings::Get().GetValue(Path, true).toBool();
    QPointer<CollapsedWidgetsViewItem> instance = this;
    QTimer::singleShot(0, [instance, visible] {
      if (instance && instance->AssociatedWidget)
      {
        instance->AssociatedWidget->setVisible(visible);
        bool v = visible;
        instance->Save(&v);
      }
    });
  }
}

void CollapsedWidgetsViewItem::Save(bool* arg)
{
  if (!Active)
    return;

  if (!AssociatedWidget)
    return;

  QSplitter* splitter = GetParentSplitterForAssociatedWidget(); 
  if (!splitter || HideInSplitter)
  {
    bool visible = arg ? *arg : IsAssociatedWidgetVisible();
    Common::Settings::Get().SetValue(Path, visible); 
  }
  if (splitter)
  {
    QString path = GetSettingsPath();
    QByteArray splitterState = splitter->saveState();
    QByteArray splitterState64 = splitterState.toBase64();
    QString stored = splitterState64;
    Common::Settings::Get().SetValue(path, stored);
  }
}

void CollapsedWidgetsViewItem::SetActive(bool active)
{
  Active = active;
}

void CollapsedWidgetsViewItem::paintEvent(QPaintEvent* /*event*/)
{
  QStylePainter painter(this);
  
  QStyleOptionButton option;
  initStyleOption(&option);
  option.rect.setWidth(height());
  option.rect.setHeight(width());

  QTransform transform = painter.transform();
  if (property("alignment").toInt() == Qt::AlignRight)
  {
    transform.translate(0, height());
    transform.rotate(-90);
  }
  else
  {
    transform.translate(width(), 0);
    transform.rotate(90);
  }
  painter.setTransform(transform);

  painter.drawControl(QStyle::CE_PushButton, option);
}

QSize CollapsedWidgetsViewItem::sizeHint() const
{
  QSize inheritedHint = QPushButton::sizeHint();
  return QSize(inheritedHint.height(), inheritedHint.width());
}

QSize CollapsedWidgetsViewItem::minimumSizeHint() const
{
  QSize inheritedHint = QPushButton::minimumSizeHint();
  return QSize(inheritedHint.height(), inheritedHint.width());
}

void CollapsedWidgetsViewItem::SetHideInSplitter(bool hidden)
{
  HideInSplitter = hidden;
  updateWidget();
}

void CollapsedWidgetsViewItem::updateWidget()
{
  if (!AssociatedWidget)
    return;

  setProperty("targetvisible", IsAssociatedWidgetVisible());
  style()->unpolish(this);
  style()->polish(this);
}

bool CollapsedWidgetsViewItem::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == AssociatedWidget)
  {
    switch (event->type())
    {
      case QEvent::Hide:
      case QEvent::Show:
      case QEvent::Resize:
        {
          QPointer<CollapsedWidgetsViewItem> ptr(this);
          QTimer::singleShot(0, [ptr](){
            if (ptr)
            {
              auto parentWidget = ptr->AssociatedWidget->parentWidget();
              if (parentWidget && parentWidget->isVisibleTo(parentWidget->window()))
                  ptr->Save(nullptr);
              ptr->updateWidget();
            }
          });
        }
        break;
      default:
        break;
    }
  }
  return false;
}

/////////////////////////////////////////////
//// Collapsed Widgets View
CollapsedWidgetsView::CollapsedWidgetsView(QWidget* parent)
  : QFrame(parent)
  , HideInSplitter(false)
{
  setProperty("alignment", Qt::AlignLeft);

#ifndef DISABLE_CYBER_FRAC_STYLE
  setStyleSheet(Style::GetCollapsedWidgetsViewStyle());
#endif

  setAutoFillBackground(true);

  setContentsMargins(0, 0, 0, 0);
  QVBoxLayout* outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);

  Layout = new QVBoxLayout();
  Layout->setSpacing(10);
  outerLayout->addSpacing(10);
  outerLayout->addItem(Layout);
  outerLayout->addSpacing(10);
  outerLayout->addStretch(1);
}

CollapsedWidgetsView::~CollapsedWidgetsView()
{
}

void CollapsedWidgetsView::SetHideInSplitter(bool hidden)
{
  HideInSplitter = hidden;
  for (const auto& button : Buttons)
    button->SetHideInSplitter(hidden);
}

CollapsedWidgetsViewItem* CollapsedWidgetsView::CreateItemForWidget(QWidget* target, const QString& name, const QString& path)
{
  CollapsedWidgetsViewItem* item = new CollapsedWidgetsViewItem(name, this, Layout, target, path, HideInSplitter);
  item->setProperty("alignment", property("alignment"));
  Layout->addWidget(item);
  Buttons << item;
  return item;
}

CollapsedWidgetsViewItem* CollapsedWidgetsView::CreateItem(const QString& name)
{
  CollapsedWidgetsViewItem* item = new CollapsedWidgetsViewItem(name, this, Layout, nullptr, QString(), HideInSplitter);
  Layout->addWidget(item);
  Buttons << item;
  return item;
}

QList<CollapsedWidgetsViewItem *> CollapsedWidgetsView::Items()
{
  return Buttons;
}

void CollapsedWidgetsView::AddStretch(int stretch)
{
  Layout->addStretch(stretch);
}

void CollapsedWidgetsView::AddSpacing(int spacing)
{
  Layout->addSpacing(spacing);
}
