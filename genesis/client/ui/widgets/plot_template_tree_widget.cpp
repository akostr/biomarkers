#include "plot_template_tree_widget.h"

#include "logic/models/plot_template_model.h"
#include "genesis_style/style.h"
#include "ui/plots/gp_items/gpshape_with_label_item.h"
#include "ui/item_models/color_item_model.h"
#include "ui/controls/pen_style_combo_box.h"
#include "ui/controls/line_ending_combo_box.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QComboBox>

namespace
{
enum {
  ColumnTitle,
  ColumnField,
  ColumnIndex,
  ColumnCount
};
}

QMap<QString, int> PlotTemplateTreeItem::m_propertiesOrder
{
    {PROP::opacity, 0},
    {PROP::transparency, 1},
    {PROP::color, 2},
    {PROP::fillColor, 3},
    {PROP::penColor, 4},
    {PROP::penStyle, 5},
    {PROP::penWidth, 6},
    {PROP::lineEndingType, 7},
    {PROP::font, 8},
    {PROP::fontSize, 9},
    {PROP::bold, 10},
    {PROP::italic, 11},
    {PROP::text, 12},
    {PROP::showOnlyAxisRect, 13},
    {PROP::lineClose, 14},
};

PlotTemplateTreeItem::PlotTemplateTreeItem(PlotTemplateItem* modelItem, QTreeWidgetItem* treeItem)
{
  m_modelItem = modelItem;
  m_treeItem = treeItem;
  updateIndex();
  addTopLevelWidget();
  //because using metaobject during initialization is bad
  QMetaObject::invokeMethod(this, &PlotTemplateTreeItem::propScan, Qt::QueuedConnection);
}

PlotTemplateItem *PlotTemplateTreeItem::modelItem()
{
  return m_modelItem;
}

void PlotTemplateTreeItem::updateIndex()
{
  m_treeItem->setData(ColumnIndex, Qt::DisplayRole, m_modelItem->index());
}

void PlotTemplateTreeItem::onPropNotify()
{
  auto senderObject = sender();
  auto senderSignalInd = senderSignalIndex();
  auto senderMetaObject = senderObject->metaObject();
  QMetaMethod notifySignal = senderMetaObject->method(senderSignalInd);
  for(int propInd = 0; propInd < senderMetaObject->propertyCount(); propInd++)
  {
    auto prop = senderMetaObject->property(propInd);
    if(!prop.hasNotifySignal())
      continue;
    if(prop.notifySignalIndex() != notifySignal.methodIndex())
      continue;
    //there is our property:
    QTreeWidgetItem* propEditorItem = nullptr;
    for(int c = 0; c < m_treeItem->childCount(); c++)
    {
      auto child = m_treeItem->child(c);
      if(child->data(ColumnTitle, Qt::UserRole).toString() == prop.name())
      {
        propEditorItem = child;
        break;
      }
    }
    auto propValue = prop.read(senderObject);
    if(propValue.isValid() && !propEditorItem)
    {//createEditor
      createEditor(prop);
    }
    else if(!propValue.isValid() && propEditorItem)
    {//removeEditor
       m_treeItem->removeChild(propEditorItem);
    }
  }
  //just example:
  // qDebug() << senderMetaObject->className() << "notifies" << notifySignal.name();
}

void PlotTemplateTreeItem::addTopLevelWidget()
{
  QString name = m_modelItem->name();
  m_treeItem->setText(ColumnTitle, name);
  QWidget *wCommon = new QWidget(m_treeItem->treeWidget());
  wCommon->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  QHBoxLayout *layout = new QHBoxLayout(wCommon);
  wCommon->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);
  layout->addStretch(1);
  auto pbSelectable = new QPushButton(wCommon);
  pbSelectable->setMaximumHeight(18);
  pbSelectable->setMaximumWidth(18);
  auto pbVisible = new QPushButton(wCommon);
  pbVisible->setMaximumHeight(18);
  pbVisible->setMaximumWidth(18);
  layout->addWidget(pbSelectable);
  layout->addWidget(pbVisible);

  auto updateVisible = [this, pbVisible](bool isVisible)
  {
    pbVisible->setIcon(isVisible ? QIcon("://resource/icons/compounds/icon_eye@2x.png") : QIcon("://resource/icons/compounds/icon_eye_close@2x.png"));
  };

  auto updateSelectable = [this, pbSelectable](bool isSelectable)
  {
    pbSelectable->setIcon(isSelectable ? QIcon("://resource/icons/compounds/icon_unlock@2x.png") : QIcon("://resource/icons/compounds/icon_lock@2x.png"));
  };

  m_treeItem->treeWidget()->setItemWidget(m_treeItem, ColumnField, wCommon);
  updateVisible(m_modelItem->isVisible());
  updateSelectable(m_modelItem->isEnabled());

  connect (pbSelectable, &QPushButton::clicked, this, [this]() {
    bool isSelectable = !m_modelItem->isEnabled();
    m_modelItem->setEnabled(isSelectable);
  });

  connect (pbVisible, &QPushButton::clicked, this, [this]() {
    bool isVisible = !m_modelItem->isVisible();
    m_modelItem->setVisible(isVisible);
  });

  connect (m_modelItem, &PlotTemplateItem::visibleChanged, this, updateVisible);
  connect (m_modelItem, &PlotTemplateItem::enabledChanged, this, updateSelectable);
}

void PlotTemplateTreeItem::addOpacity()
{
  QWidget *opacityWidget = new QWidget(m_treeItem->treeWidget());
  opacityWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  opacityWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(opacityWidget);
  opacityWidget->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);
  auto opacitySlider = new QSlider(opacityWidget);
  opacitySlider->setOrientation(Qt::Horizontal);
  opacitySlider->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qslider.qss"));
  opacitySlider->setMinimum(0);
  opacitySlider->setMaximum(100);
  layout->addWidget(opacitySlider);

  auto opacityLabel = new QLabel(opacityWidget);

  layout->addWidget(opacityLabel);
  layout->setStretch(0,1);

  connect(opacitySlider, &QSlider::valueChanged, opacityLabel, [opacityLabel](int value)
          {
            opacityLabel->setText(QString("%1%").arg(value));
          });

  QTreeWidgetItem* opacityItem = new QTreeWidgetItem(m_treeItem);
  m_treeItem->addChild(opacityItem);
  opacityItem->setText(ColumnTitle, tr("Opacity"));
  opacityItem->setData(ColumnTitle, Qt::UserRole, PROP::opacity);
  opacitySlider->setValue(std::round(m_modelItem->property(PROP::opacity).toDouble() * 100));
  opacityItem->treeWidget()->setItemWidget(opacityItem, ColumnField, opacityWidget);
  connect (opacitySlider, &QSlider::valueChanged, this, [this](int value)
          {
            m_modelItem->setProperty(PROP::opacity, value / 100.);
          });
}

void PlotTemplateTreeItem::addShowOnlyAxesRect()
{
  auto showOnlyRectItem = new QTreeWidgetItem(m_treeItem);
  m_treeItem->addChild(showOnlyRectItem);
  showOnlyRectItem->setText(ColumnTitle, tr("Show only axes rect"));
  showOnlyRectItem->setData(ColumnTitle, Qt::UserRole, PROP::showOnlyAxisRect);
  showOnlyRectItem->setCheckState(ColumnField, m_modelItem->showOnlyAxisRect() ? Qt::Checked : Qt::Unchecked);
  connect(showOnlyRectItem->treeWidget(), &QTreeWidget::itemChanged, this, [this, showOnlyRectItem](QTreeWidgetItem *item, int column) {
    if (item == showOnlyRectItem && column == ColumnField &&
        m_modelItem->showOnlyAxisRect() != (showOnlyRectItem->checkState(ColumnField) != Qt::Unchecked))
    {
      bool onlyRect = showOnlyRectItem->checkState(ColumnField) != Qt::Unchecked;
      m_modelItem->setShowOnlyAxisRect(onlyRect);
    }
  });
}

void PlotTemplateTreeItem::addColor()
{
  auto mo = m_modelItem->metaObject();
  int propertyIndex = mo->indexOfProperty(PROP::color);
  QColor color = mo->property(propertyIndex).read(m_modelItem).value<QColor>();

  auto colorItem = new QTreeWidgetItem(m_treeItem);
  m_treeItem->addChild(colorItem);
  colorItem->setText(ColumnTitle, tr("Color"));
  colorItem->setData(ColumnTitle, Qt::UserRole, PROP::color);

  QWidget *colorWidget = new QWidget(m_treeItem->treeWidget());
  colorWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  colorWidget->setAttribute(Qt::WA_StyledBackground, false);
  colorWidget->setMinimumHeight(60);
  QVBoxLayout *layout = new QVBoxLayout(colorWidget);
  colorWidget->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);

  QComboBox *colorComboBox = new QComboBox(colorWidget);
  auto colorModel = new ColorItemModel(colorComboBox);
  colorModel->showOnlyColor(true);
  colorComboBox->setModel(colorModel);
  {
    auto currInd = colorModel->getIndex(color);
    if(currInd == GPShapeItem::CustomColor)
      currInd = 0;
    colorComboBox->setCurrentIndex(currInd);
  }
  layout->addWidget(colorComboBox);
  layout->setStretch(0,0);
  layout->addStretch(1);

  colorItem->treeWidget()->setItemWidget(colorItem, ColumnField, colorWidget);
  connect(colorComboBox, &QComboBox::currentIndexChanged, this, [this, colorComboBox] {
    auto mo = m_modelItem->metaObject();
    int propertyIndex = mo->indexOfProperty(PROP::color);
    QVariant color = colorComboBox->currentData(Qt::UserRole);
    mo->property(propertyIndex).write(m_modelItem, color);
  });
}

void PlotTemplateTreeItem::addFont()
{
  auto mo = m_modelItem->metaObject();
  int propertyIndex = mo->indexOfProperty(PROP::font);
  QVariant fontVariant = mo->property(propertyIndex).read(m_modelItem);
  QFont font = fontVariant.value<QFont>();

  auto sizeItem = new QTreeWidgetItem(m_treeItem);
  sizeItem->setText(ColumnTitle, tr("Size"));
  QWidget *sizeWidget = new QWidget(m_treeItem->treeWidget());
  sizeWidget->setMinimumHeight(60);
  sizeWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  sizeWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *sizeLayout = new QVBoxLayout(sizeWidget);
  sizeLayout->setContentsMargins(0,0,0,0);

  sizeWidget->setLayout(sizeLayout);
  sizeLayout->setContentsMargins(0,0,0,0);
  QSpinBox *sizeSpinbox = new QSpinBox(sizeWidget);
  sizeSpinbox->setRange(1, 124);
  sizeSpinbox->setValue(font.pointSize());
  sizeLayout->addWidget(sizeSpinbox);
  sizeLayout->setStretch(0,0);
  sizeLayout->addStretch(1);
  sizeItem->treeWidget()->setItemWidget(sizeItem, ColumnField, sizeWidget);

  auto boldItem = new QTreeWidgetItem(m_treeItem);
  boldItem->setText(ColumnTitle, tr("Bold"));
  boldItem->setCheckState(ColumnField, font.bold() ? Qt::Checked : Qt::Unchecked);

  auto italicItem = new QTreeWidgetItem(m_treeItem);
  italicItem->setText(ColumnTitle, tr("Italic"));
  italicItem->setCheckState(ColumnField, font.italic() ? Qt::Checked : Qt::Unchecked);

  auto updateModelFont = [this, sizeSpinbox, boldItem, italicItem]()
  {
    auto mo = m_modelItem->metaObject();
    int propertyIndex = mo->indexOfProperty(PROP::font);
    QVariant fontVariant = mo->property(propertyIndex).read(m_modelItem);
    QFont font = fontVariant.value<QFont>();
    font.setPointSize(sizeSpinbox->value());
    font.setBold(boldItem->checkState(ColumnField) != Qt::Unchecked);
    font.setItalic(italicItem->checkState(ColumnField) != Qt::Unchecked);
    mo->property(propertyIndex).write(m_modelItem, font);
  };

  connect(sizeSpinbox, &QSpinBox::valueChanged, this, updateModelFont);

  connect(m_treeItem->treeWidget(), &QTreeWidget::itemChanged, this, [this, boldItem, italicItem, updateModelFont](QTreeWidgetItem *item, int column) {
    if (column == ColumnField && (item == boldItem || item == italicItem))
    {
      updateModelFont();
    }
  });
}

void PlotTemplateTreeItem::addUnifiedColorEditor(const char *propName, const QString &propTitle)
{
  auto mo = m_modelItem->metaObject();
  int propertyIndex = mo->indexOfProperty(propName);
  QColor color = mo->property(propertyIndex).read(m_modelItem).value<QColor>();

  auto colorItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), colorItem);
  // m_treeItem->addChild(colorItem);
  colorItem->setText(ColumnTitle, propTitle);
  colorItem->setData(ColumnTitle, Qt::UserRole, propName);
  QWidget *colorWidget = new QWidget(m_treeItem->treeWidget());
  colorWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  colorWidget->setAttribute(Qt::WA_StyledBackground, false);
  colorWidget->setMinimumHeight(60);
  QVBoxLayout *layout = new QVBoxLayout(colorWidget);
  colorWidget->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);

  QComboBox *colorComboBox = new QComboBox(m_treeItem->treeWidget());
  auto colorModel = new ColorItemModel(colorComboBox);
  colorModel->showOnlyColor(true);
  colorComboBox->setModel(colorModel);
  {
    auto currInd = colorModel->getIndex(color);
    if(currInd == GPShapeItem::CustomColor)
      currInd = 0;
    colorComboBox->setCurrentIndex(currInd);
  }
  layout->addWidget(colorComboBox);
  layout->addStretch(1);
  colorItem->treeWidget()->setItemWidget(colorItem, ColumnField, colorWidget);

  connect(colorComboBox, &QComboBox::currentIndexChanged, this, [this, colorComboBox, propName] {
    auto mo = m_modelItem->metaObject();
    int propertyIndex = mo->indexOfProperty(propName);
    QVariant color = colorComboBox->currentData(Qt::UserRole);
    mo->property(propertyIndex).write(m_modelItem, color);
  });
}

void PlotTemplateTreeItem::addUnifiedCheckbox(const char *propName, const QString &propTitle)
{
  auto mo = m_modelItem->metaObject();
  int propertyIndex = mo->indexOfProperty(propName);
  bool checked = mo->property(propertyIndex).read(m_modelItem).toBool();

  auto checkboxItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), checkboxItem);
  checkboxItem->setText(ColumnTitle, propTitle);
  checkboxItem->setData(ColumnTitle, Qt::UserRole, propName);
  checkboxItem->setCheckState(ColumnField, checked ? Qt::Checked : Qt::Unchecked);
  connect(checkboxItem->treeWidget(), &QTreeWidget::itemChanged, this, [this, checkboxItem, propName](QTreeWidgetItem *item, int column) {
    if (item == checkboxItem && column == ColumnField)
    {
      auto mo = m_modelItem->metaObject();
      int propertyIndex = mo->indexOfProperty(propName);
      bool onlyRect = checkboxItem->checkState(ColumnField) != Qt::Unchecked;
      mo->property(propertyIndex).write(m_modelItem, onlyRect);
    }
  });
}

void PlotTemplateTreeItem::addUnifiedSlider(const char *propName, const QString &propTitle, const QString &suffix, int min, int max, bool fromZeroToOneNormalized)
{
  QWidget *sliderWidget = new QWidget(m_treeItem->treeWidget());
  sliderWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  sliderWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(sliderWidget);
  sliderWidget->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);
  auto slider = new QSlider(sliderWidget);
  slider->setOrientation(Qt::Horizontal);
  slider->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qslider.qss"));
  slider->setMinimum(min);
  slider->setMaximum(max);
  layout->addWidget(slider);

  auto label = new QLabel(sliderWidget);
  connect(slider, &QSlider::valueChanged, label, [label, suffix](int value)
          {
            label->setText(QString("%1%2").arg(value).arg(suffix));
          });

  layout->addWidget(label);
  layout->setStretch(0,1);

  QTreeWidgetItem* sliderItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), sliderItem);
  sliderItem->setText(ColumnTitle, propTitle);
  sliderItem->setData(ColumnTitle, Qt::UserRole, propName);

  auto propertyValue = m_modelItem->property(propName);
  if(fromZeroToOneNormalized)
    slider->setValue(std::round(propertyValue.toDouble() * 100));
  else
    slider->setValue(propertyValue.toInt());

  sliderItem->treeWidget()->setItemWidget(sliderItem, ColumnField, sliderWidget);
  connect (slider, &QSlider::valueChanged, this, [this, propName, fromZeroToOneNormalized](int value)
          {
            if(fromZeroToOneNormalized)
              m_modelItem->setProperty(propName, value / 100.);
            else
              m_modelItem->setProperty(propName, value);
          });
}

void PlotTemplateTreeItem::addUnifiedSpinBox(const char *propName, const QString &propTitle, const QString &suffix, int min, int max)
{
  QWidget *spinboxWidget = new QWidget(m_treeItem->treeWidget());
  spinboxWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  spinboxWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(spinboxWidget);
  spinboxWidget->setLayout(layout);
  spinboxWidget->setMinimumHeight(60);
  layout->setContentsMargins(0,0,0,0);

  auto spinbox = new QSpinBox(spinboxWidget);
  spinbox->setSuffix(suffix);
  spinbox->setMinimum(min);
  spinbox->setMaximum(max);
  layout->addWidget(spinbox);
  layout->addStretch(1);

  QTreeWidgetItem* spinboxItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), spinboxItem);
  spinboxItem->setText(ColumnTitle, propTitle);
  spinboxItem->setData(ColumnTitle, Qt::UserRole, propName);

  auto propertyValue = m_modelItem->property(propName);
  spinbox->setValue(propertyValue.toInt());

  spinboxItem->treeWidget()->setItemWidget(spinboxItem, ColumnField, spinboxWidget);
  connect (spinbox, &QSpinBox::valueChanged, this, [this, propName](int value)
          {
            m_modelItem->setProperty(propName, value);
          });
}

void PlotTemplateTreeItem::addUnifiedFont(const char *propName, const QString &propTitle)
{
  // auto mo = m_modelItem->metaObject();
  // int propertyIndex = mo->indexOfProperty(propName);
  // QVariant fontVariant = mo->property(propertyIndex).read(m_modelItem);
  // QFont font = fontVariant.value<QFont>();

  // // auto sizeItem = new QTreeWidgetItem();
  // // m_treeItem->insertChild(getBeforeItemIndex(propName), sizeItem);
  // // sizeItem->setText(ColumnTitle, tr("Size"));
  // // sizeItem->setData(ColumnTitle, Qt::UserRole, propTitle);
  // // QWidget *sizeWidget = new QWidget(m_treeItem->treeWidget());
  // // sizeWidget->setMinimumHeight(60);
  // // sizeWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  // // sizeWidget->setAttribute(Qt::WA_StyledBackground, false);
  // // QVBoxLayout *sizeLayout = new QVBoxLayout(sizeWidget);
  // // sizeLayout->setContentsMargins(0,0,0,0);

  // // sizeWidget->setLayout(sizeLayout);
  // // sizeLayout->setContentsMargins(0,0,0,0);
  // // QSpinBox *sizeSpinbox = new QSpinBox(sizeWidget);
  // // sizeSpinbox->setRange(1, 124);
  // // sizeSpinbox->setValue(font.pointSize());
  // // sizeLayout->addWidget(sizeSpinbox);
  // // sizeLayout->setStretch(0,0);
  // // sizeLayout->addStretch(1);
  // // sizeItem->treeWidget()->setItemWidget(sizeItem, ColumnField, sizeWidget);

  // // auto boldItem = new QTreeWidgetItem(m_treeItem);
  // // boldItem->setText(ColumnTitle, tr("Bold"));
  // // boldItem->setData(ColumnTitle, Qt::UserRole, propTitle);
  // // boldItem->setCheckState(ColumnField, font.bold() ? Qt::Checked : Qt::Unchecked);

  // // auto italicItem = new QTreeWidgetItem(m_treeItem);
  // // italicItem->setText(ColumnTitle, tr("Italic"));
  // // italicItem->setData(ColumnTitle, Qt::UserRole, propTitle);
  // // italicItem->setCheckState(ColumnField, font.italic() ? Qt::Checked : Qt::Unchecked);

  // auto updateModelFont = [this, /*sizeSpinbox, boldItem, italicItem,*/ propName]()
  // {
  //   auto mo = m_modelItem->metaObject();
  //   int propertyIndex = mo->indexOfProperty(propName);
  //   QVariant fontVariant = mo->property(propertyIndex).read(m_modelItem);
  //   QFont font = fontVariant.value<QFont>();
  //   // font.setPointSize(sizeSpinbox->value());
  //   // font.setBold(boldItem->checkState(ColumnField) != Qt::Unchecked);
  //   // font.setItalic(italicItem->checkState(ColumnField) != Qt::Unchecked);
  //   mo->property(propertyIndex).write(m_modelItem, font);
  // };

  // // connect(sizeSpinbox, &QSpinBox::valueChanged, this, updateModelFont);

  // connect(m_treeItem->treeWidget(),
  //         &QTreeWidget::itemChanged,
  //         this,
  //         [this, /*boldItem, italicItem,*/ updateModelFont](QTreeWidgetItem *item, int column) {
  //           if (column == ColumnField && (item == boldItem || item == italicItem))
  //           {
  //             updateModelFont();
  //           }
  //         });
}

void PlotTemplateTreeItem::addUnifiedText(const char* propName, const QString& propTitle)
{
  QWidget *textWidget = new QWidget(m_treeItem->treeWidget());
  textWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  textWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(textWidget);
  textWidget->setLayout(layout);
  layout->setContentsMargins(0,0,0,0);

  QTextEdit *textEdit = new QTextEdit(textWidget);
  layout->addWidget(textEdit);
  layout->addStretch(1);
  auto propertyValue = m_modelItem->property(propName);
  QString text = propertyValue.toString();
  textEdit->setText(text);

  QTreeWidgetItem* textItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), textItem);
  textItem->setText(ColumnTitle, propTitle);
  textItem->setData(ColumnTitle, Qt::UserRole, propName);
  textItem->treeWidget()->setItemWidget(textItem, ColumnField, textWidget);
  connect (textEdit, &QTextEdit::textChanged, this, [this, propName, textEdit](){
    m_modelItem->setProperty(propName, textEdit->toPlainText());
  });
}

void PlotTemplateTreeItem::addUnifiedPenStyleCombo(const char *propName, const QString &propTitle)
{
  QWidget *comboWidget = new QWidget(m_treeItem->treeWidget());
  comboWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  comboWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(comboWidget);
  comboWidget->setLayout(layout);
  comboWidget->setMinimumHeight(60);
  layout->setContentsMargins(0,0,0,0);

  auto combo = new PenStyleComboBox(comboWidget);
  layout->addWidget(combo);
  layout->addStretch(1);

  QTreeWidgetItem* comboItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), comboItem);
  comboItem->setText(ColumnTitle, propTitle);
  comboItem->setData(ColumnTitle, Qt::UserRole, propName);

  auto propertyValue = m_modelItem->property(propName);
  combo->setCurrentPenStyle(propertyValue.value<Qt::PenStyle>());

  comboItem->treeWidget()->setItemWidget(comboItem, ColumnField, comboWidget);
  connect (combo, &PenStyleComboBox::penStyleChanged, this, [this, propName](Qt::PenStyle style)
          {
            m_modelItem->setProperty(propName, QVariant::fromValue(style));
          });
}

void PlotTemplateTreeItem::addUnifiedLineEndingStyleCombo(const char *propName, const QString &propTitle)
{
// LineEndingComboBox
  QWidget *comboWidget = new QWidget(m_treeItem->treeWidget());
  comboWidget->setStyleSheet(Style::GetStyleSheet(":/resource/styles/treewidget_qwidget.qss"));
  comboWidget->setAttribute(Qt::WA_StyledBackground, false);
  QVBoxLayout *layout = new QVBoxLayout(comboWidget);
  comboWidget->setLayout(layout);
  comboWidget->setMinimumHeight(60);
  layout->setContentsMargins(0,0,0,0);

  auto combo = new LineEndingComboBox(comboWidget);
  layout->addWidget(combo);
  layout->addStretch(1);

  QTreeWidgetItem* comboItem = new QTreeWidgetItem();
  m_treeItem->insertChild(getBeforeItemIndex(propName), comboItem);
  comboItem->setText(ColumnTitle, propTitle);
  comboItem->setData(ColumnTitle, Qt::UserRole, propName);

  auto propertyValue = m_modelItem->property(propName);
  combo->setCurrentEndingStyle(propertyValue.value<GPLineEnding::EndingStyle>());

  comboItem->treeWidget()->setItemWidget(comboItem, ColumnField, comboWidget);
  connect (combo, &LineEndingComboBox::lineEndingChanged, this, [this, propName](GPLineEnding::EndingStyle style)
          {
            m_modelItem->setProperty(propName, QVariant::fromValue(style));
          });
}

void PlotTemplateTreeItem::propScan()
{
  auto mo = m_modelItem->metaObject();
  auto propOffset = mo->propertyOffset();//for class-specific props
  auto propCount = mo->propertyCount();

  for(int pi = 0; pi < propCount; pi++)
  {
    auto prop = mo->property(pi);
    if(prop.hasNotifySignal())
    {
      QMetaMethod signal = prop.notifySignal();
      QMetaMethod updateSlot = metaObject()->method(
          metaObject()->indexOfSlot("onPropNotify()"));
      bool connected = connect(m_modelItem, signal, this, updateSlot);
      // qDebug() << signal.name() << "is " << (connected ? "connected to" : "NOT connected to") << updateSlot.name();
    }
    createEditor(prop);
  }

  // connect(m_modelItem, &PlotTemplateItem::propertyStateChanged, this, [this, createEditor, removeEditor](const char* propName, bool isActive)
          // {
          //   if(isActive)
          //   {
          //     auto mo = m_modelItem->metaObject();
          //     auto propInd = mo->indexOfProperty(propName);
          //     auto prop = mo->property(propInd);
          //     createEditor(prop);
          //   }
          //   else
          //   {
          //     removeEditor(propName);
          //   }
          // });
}

QString PlotTemplateTreeItem::trPropName(const char *propName)
{
  return QCoreApplication::translate("PROP", PROP::mPropTitles.value(propName, PROP::mPropTitles[PROP::unknown]));
}

void PlotTemplateTreeItem::createEditor(QMetaProperty &prop)
{
  auto typeId = prop.metaType().id();
  if(typeId == QMetaType::fromType<QColor>().id()
      || strcmp(prop.name(), PROP::fillColor) == 0)
  {
    if(prop.read(m_modelItem).isValid())
    {
      addUnifiedColorEditor(prop.name(),
                            trPropName(prop.name()));
    }
  }
  else if(strcmp(prop.name(), PROP::lineEndingType) == 0 && prop.read(m_modelItem).isValid())
  {
    addUnifiedLineEndingStyleCombo(prop.name(), trPropName(prop.name()));
  }
  else if(typeId == QMetaType::fromType<Qt::PenStyle>().id())
  {
    addUnifiedPenStyleCombo(prop.name(), trPropName(prop.name()));
  }
  else if(typeId == QMetaType::fromType<bool>().id())
  {
    if(m_propertiesOrder.contains(prop.name())
        && prop.read(m_modelItem).isValid())
    {
      addUnifiedCheckbox(prop.name(), trPropName(prop.name()));
    }
  }
  else if((strcmp(prop.name(), PROP::penWidth) == 0
              || (typeId == QMetaType::fromType<int>().id()
                  && m_propertiesOrder.contains(prop.name())))
             && prop.read(m_modelItem).isValid())
  {
    if(strcmp(prop.name(), PROP::penWidth) == 0)
    {
      addUnifiedSpinBox(prop.name(),
                        trPropName(prop.name()),
                        "px",
                        1,
                        10);
    }
    else if(strcmp(prop.name(), PROP::fontSize) == 0)
    {
      addUnifiedSpinBox(prop.name(),
                        trPropName(prop.name()),
                        "px",
                        1,
                        200);
    }
  }
  else if(strcmp(prop.name(), PROP::opacity) == 0
           || strcmp(prop.name(), PROP::transparency) == 0)
  {
    if(prop.read(m_modelItem).isValid())
    {
      addUnifiedSlider(prop.name(),
                       trPropName(prop.name()),
                       "%",
                       0,
                       100,
                       true);
    }
  }
  // else if(strcmp(prop.name(), PROP::font) == 0)
  // {
  //   addUnifiedFont(prop.name(), trPropName(prop.name()));
  // }
  else if(strcmp(prop.name(), PROP::text) == 0)
  {
    addUnifiedText(prop.name(), trPropName(prop.name()));
  }
}

void PlotTemplateTreeItem::removeEditor(const char *propName)
{
  for(int i = 0; i < m_treeItem->childCount(); i++)
  {
    auto child = m_treeItem->child(i);
    if(child->data(ColumnTitle, Qt::UserRole).toString() == propName)
    {
      m_treeItem->removeChild(child);
      return;
    }
  }
}

int PlotTemplateTreeItem::getBeforeItemIndex(QString propName)
{
  auto currPropOrder = m_propertiesOrder.value(propName, -1);
  if(currPropOrder == -1)
    return m_treeItem->childCount();
  for(int i = 0; i < m_treeItem->childCount(); i++)
  {
    auto child = m_treeItem->child(i);
    auto childPropName = child->data(ColumnTitle, Qt::UserRole).toString();
    auto childPropOrder = m_propertiesOrder.value(childPropName, -1);
    if(childPropOrder == -1)
      return i;
    if(childPropOrder >= currPropOrder)
      return i;
  }
  return m_treeItem->childCount();
}
//--------------------------------------------------------------------

PlotTemplateTreeWidget::PlotTemplateTreeWidget(QWidget *parent)
  : QTreeWidget(parent)
  , m_model(nullptr)
{
  //doesn't work with tree widget
  // setTextElideMode(Qt::TextElideMode::ElideNone);
  // setWordWrap(true);
  headerItem()->setHidden(true);
  setColumnCount(ColumnCount);
  hideColumn(2);
  setSortingEnabled(true);
  header()->setStretchLastSection(false);
  sortByColumn(2, Qt::AscendingOrder);

  header()->setSectionResizeMode(0, QHeaderView::Fixed);
  header()->setSectionResizeMode(1, QHeaderView::Fixed);
  header()->resizeSection(0, 250);
  header()->resizeSection(1, 150);
  setFixedWidth(250 + 150 + 20);//20 for scrollbar
}

void PlotTemplateTreeWidget::setModel(PlotTemplateModel *model)
{
  if (model)
  {
    if (m_model)
      disconnect(m_model, 0, this, 0);
    clear();//clear tree when model changes
    m_model = model;
    connect(m_model, &PlotTemplateModel::itemAdded, this, &PlotTemplateTreeWidget::addItem);
    connect(m_model, &PlotTemplateModel::itemRemoved, this, &PlotTemplateTreeWidget::removeItem);
    connect(m_model, &PlotTemplateModel::rowsMoved, this, &PlotTemplateTreeWidget::updateIndexes);
    if(auto root = m_model->getRootItem())
    {//actualize tree if there is some items
      for(auto& item : m_model->getRootItem()->items())
        addItem(item);
    }
  }
}

PlotTemplateItem *PlotTemplateTreeWidget::getModelItem(QTreeWidgetItem* treeItem)
{
  if (m_items.contains(treeItem))
    return m_items.value(treeItem)->modelItem();
  return nullptr;
}


void PlotTemplateTreeWidget::addItem(PlotTemplateItem* modelItem)
{
  QTreeWidgetItem *newTreeItem = new QTreeWidgetItem(this);
  addTopLevelItem(newTreeItem);
  PlotTemplateTreeItem * plotTemplateItem = new PlotTemplateTreeItem(modelItem, newTreeItem);
  m_items.insert(newTreeItem, plotTemplateItem);
  updateIndexes();
}

void PlotTemplateTreeWidget::removeItem(PlotTemplateItem* modelItem)
{
  for (auto it = m_items.begin(); it != m_items.end(); it++)
  {
    if (it.value()->modelItem() == modelItem)
    {
      delete it.value();
      delete it.key();
      m_items.remove(it.key());
      updateIndexes();
      return;
    }
  }
}

void PlotTemplateTreeWidget::updateIndexes()
{
  setSortingEnabled(false);
  for(auto item: m_items.values())
  {
    item->updateIndex();
  }
  setSortingEnabled(true);
}

void PlotTemplateTreeWidget::updateWidth()
{
  int columnsWidth = 0;
  for(int i = 0; i < header()->count(); i++)
  {
    columnsWidth += header()->sectionSize(i);
  }
  qDebug() << "CURRENT WIDTH" << width() << "HEADER WIDTH" << header()->width() << "COLUMNS SUMMARY WIDTH" << columnsWidth;

  setFixedWidth(columnsWidth + indentation());
  qDebug() << "NEW CURRENT WIDTH" << width();
}


