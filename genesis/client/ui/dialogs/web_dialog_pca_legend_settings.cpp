#include "web_dialog_pca_legend_settings.h"

#include <genesis_style/style.h>
#include <ui/plots/gp_items/sample_info_plate.h>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QFormLayout>
#include <QElapsedTimer>
#include <QUuid>

namespace Dialogs
{
WebDialogPCALegendSettings::WebDialogPCALegendSettings(QWidget *parent, CountsPlot* plot)
  : Templates::Dialog(parent, QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::Cancel)
  , Plot(plot)
{
//  timer.start();
  Q_ASSERT(Plot);
  if(!Plot)
  {
    //qDebug() << "Plot pointer is null";
    return;
  }
  Size = QSizeF(0.5, 0);
  SetupUi();
  auto s = Templates::Dialog::Settings();
  s.dialogHeader = tr("Edit legend");
  s.buttonsNames = {{QDialogButtonBox::Ok, tr("Apply")}};
  s.buttonsProperties = {{QDialogButtonBox::Ok, {{"blue", true}}}};
  applySettings(s);
}

void WebDialogPCALegendSettings::Accept()
{
  if(!DontChangeUserStyleCheckBox->isChecked())
  {
    CustomItemStyleMap customStyle = Plot->getCustomItemStyle();
    QList<GPShapeWithLabelItem*> customItems;
    for (auto key : customStyle.keys())
    {
      auto plotItem = Plot->findItemByFileId(key);
      if(plotItem)
      {
        customItems.append(plotItem);
      }
    }
    Plot->ResetItemStyleToDefault(customItems);
  }

  Plot->setShapeStyle(ShapeStyle);
  Plot->setColorStyle(ColorStyle);
  Plot->UpdateItemsSizes();
  Plot->UpdateItemsShapesBy((ParameterType)ShapeGroupCombo->currentData().toInt(), false);
  Plot->UpdateItemsColorsBy((ParameterType)ColorGroupCombo->currentData().toInt());

  WebDialog::Accept();
}

void WebDialogPCALegendSettings::SetupUi()
{
  auto content = getContent();
  auto body = new QWidget(content);
  body->setContentsMargins(0,0,0,0);
  content->layout()->addWidget(body);
  DontChangeUserStyleCheckBox = new QCheckBox(tr("Don't change users style"), this);
  DontChangeUserStyleCheckBox->setTristate(false);
  ButtonLayout->insertWidget(0, DontChangeUserStyleCheckBox);
  ButtonLayout->insertSpacerItem(1, new QSpacerItem(32,0,QSizePolicy::Fixed, QSizePolicy::Fixed));
  DontChangeUserStyleCheckBox->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qcheckbox.qss"));
//  {
//    auto m = body->contentsMargins();
//    m.setBottom(0);
//    body->setContentsMargins(m);
//  }
//  Content->layout()->addWidget(body);
//  {
//    auto m = Content->contentsMargins();
//    m.setBottom(0);
//    Content->setContentsMargins(m);
//  }

  auto l = new QVBoxLayout();
  l->setContentsMargins(0,0,0,0);
  body->setLayout(l);

  auto headLayout = new QHBoxLayout();
  l->addLayout(headLayout);

//  auto caption = new QLabel(tr("Edit legend"));
//  caption->setStyleSheet(Style::Genesis::GetH2());
//  headLayout->addWidget(caption);
//  headLayout->addStretch();

//  auto closeButton = new QPushButton();
//  closeButton->setIcon(QIcon(":/resource/icons/icon_action_cross.png"));
//  closeButton->setFlat(true);
//  headLayout->addWidget(closeButton);
//  connect(closeButton, &QPushButton::clicked, this, &WebDialog::Reject);

  TabWidget = new QTabWidget(body);
//  {
//    auto m = TabWidget->contentsMargins();
//    m.setBottom(0);
//    TabWidget->setContentsMargins(m);
//  }
//  TabWidget->setStyleSheet(Style::Genesis::GetUiStyle());
  l->addWidget(TabWidget);
  {//// Shape tab
    ShapeTab = new QScrollArea(TabWidget);
    ShapeTab->setAutoFillBackground(false);
    ShapeTab->setWidgetResizable(true);
    ShapeTab->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    ShapeTab->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    TabWidget->addTab(ShapeTab, tr("Parameter - figure"));

    auto shapeWidgetContainer = new QWidget(ShapeTab);
    shapeWidgetContainer->setContentsMargins(0,0,0,0);

    ShapeTab->setWidget(shapeWidgetContainer);

    auto verticalShapeLayout = new QVBoxLayout();
    verticalShapeLayout->setSizeConstraint(QLayout::SetMinimumSize);
    shapeWidgetContainer->setLayout(verticalShapeLayout);
    auto mainGrid = new QGridLayout;
    verticalShapeLayout->addLayout(mainGrid);

    ShapeGroupCombo = new QComboBox(ShapeTab);
    ComboIconSize = ShapeGroupCombo->iconSize();
    SetupShapes();
    SetupColors();
    auto mainLabel = new QLabel(tr("Shape grouping parameter"));
    mainLabel->setStyleSheet(Style::ApplySASS("QLabel{font:  @h4;color: @textColor;}"));

    AddRow(mainGrid, mainLabel, ShapeGroupCombo);

    QFrame* line = new QFrame(ShapeTab);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("QFrame { border-top: 1px solid #ebedef; }");
    mainGrid->addWidget(line, mainGrid->rowCount(), 0, 1, 3);

    ShapeStackedLayout = new QStackedLayout;
    ShapeStackedLayout->setSizeConstraint(QLayout::SetMinimumSize);
    verticalShapeLayout->addLayout(ShapeStackedLayout);

    auto emptyShapeWidget = new QWidget(ShapeTab);
    emptyShapeWidget->setProperty("parameter", ParameterType::LastType);
    auto emptyShapeLayout = new QVBoxLayout;
    emptyShapeWidget->setLayout(emptyShapeLayout);
    auto emptyLabel = new QLabel(tr("Shape grouping parameter is not set"));
    emptyLabel->setProperty("gray", true);
    emptyShapeLayout->addWidget(emptyLabel);
    emptyShapeLayout->addStretch();
    ShapeStackedLayout->addWidget(emptyShapeWidget);

    ShapeStyle = Plot->getShapeStyle();
    for(auto paramKey : ShapeStyle.keys())
    {
      auto content = new QWidget(shapeWidgetContainer);
      content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      content->setContentsMargins(0,0,0,0);
      auto vl = new QVBoxLayout;
      vl->setContentsMargins(0,0,0,0);
//      vl->setSpacing(0);
      content->setLayout(vl);
      auto grid = new QGridLayout;
//      grid->setSpacing(0);
      vl->addLayout(grid);
      vl->addStretch(1);
      ShapeStackedLayout->addWidget(content);
      content->setProperty("parameter", paramKey);

      auto shapeLabelCaption = new QLabel(ParameterToString(paramKey));
      shapeLabelCaption->setStyleSheet(Style::Genesis::GetH4());
      auto shapeComboCaption = new QLabel(tr("Shape"));
      shapeComboCaption->setStyleSheet(Style::Genesis::GetH4());
      AddRow(grid, shapeLabelCaption, shapeComboCaption);

      auto fillComboAndSet = [this](QComboBox* combo, GPShape::ShapeType shape)
      {
        int ind = 0;
        for(auto& s : ShapesList)
        {
          if(s.second == shape)
            ind = combo->count();
          combo->addItem(s.first, GPShape::ToString(s.second), s.second);
        }
        combo->setCurrentIndex(ind);
      };

      for(auto val : ShapeStyle[paramKey].keys())
      {
        auto shape = ShapeStyle[paramKey][val];
        auto combo = new QComboBox;
        fillComboAndSet(combo, shape);
        connect(combo, &QComboBox::currentIndexChanged, this, [this, val, paramKey, combo](int ind)
        {
          ShapeStyle[paramKey][val] = (GPShape::ShapeType)combo->itemData(ind).toInt();
        });
        AddRow(grid, val, combo);
      }
    }
    auto shapeGroupParam = Plot->GetShapeSortParam();
    int currentGroupInd = 0;
    ShapeGroupCombo->addItem(tr("Don't group by shapes"), ParameterType::LastType);
    for(int i = 0; i < ParameterType::LastType; i++)
    {
      auto param = (ParameterType)i;
      if(param == shapeGroupParam)
        currentGroupInd = ShapeGroupCombo->count();
      ShapeGroupCombo->addItem(ParameterToString(param), param);
    }

    connect(ShapeGroupCombo, &QComboBox::currentIndexChanged, this, [this](int ind)
    {
      auto param = (ParameterType)(ShapeGroupCombo->itemData(ind).toInt());
      SetStackedLayoutIndex(ShapeStackedLayout, param);
    });

    ShapeGroupCombo->setCurrentIndex(currentGroupInd);
  }

  {//// Color tab
    ColorTab = new QScrollArea(TabWidget);
    ColorTab->setAutoFillBackground(false);
    ColorTab->setWidgetResizable(true);
    ColorTab->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    TabWidget->addTab(ColorTab, tr("Parameter - color"));

    auto colorWidgetContainer = new QWidget(ColorTab);
    colorWidgetContainer->setContentsMargins(0,0,0,0);

    ColorTab->setWidget(colorWidgetContainer);

    auto verticalColorLayout = new QVBoxLayout();
    colorWidgetContainer->setLayout(verticalColorLayout);
    auto mainGrid = new QGridLayout;
    verticalColorLayout->addLayout(mainGrid);

    ColorGroupCombo = new QComboBox();
    auto mainLabel = new QLabel(tr("Color grouping parameter"));
    mainLabel->setStyleSheet(Style::ApplySASS("QLabel{font:  @h4;color: @textColor;}"));
    AddRow(mainGrid, mainLabel, ColorGroupCombo);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("QFrame { border-top: 1px solid #ebedef; }");
    mainGrid->addWidget(line, mainGrid->rowCount(), 0, 1, 3);

    ColorStackedLayout = new QStackedLayout;
    verticalColorLayout->addLayout(ColorStackedLayout);

    auto emptyColorWidget = new QWidget();
    emptyColorWidget->setProperty("parameter", ParameterType::LastType);
    auto emptyColorLayout = new QVBoxLayout;
    emptyColorWidget->setLayout(emptyColorLayout);
    auto emptyLabel = new QLabel(tr("Color grouping parameter is not set"));
    emptyLabel->setProperty("gray", true);
    emptyColorLayout->addWidget(emptyLabel);
    emptyColorLayout->addStretch();
    ColorStackedLayout->addWidget(emptyColorWidget);

    ColorStyle = Plot->getColorStyle();
    for(auto paramKey : ColorStyle.keys())
    {
      auto content = new QWidget;
      content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
      content->setContentsMargins(0,0,0,0);
      auto vl = new QVBoxLayout;
      vl->setContentsMargins(0,0,0,0);
//      vl->setSpacing(0);
      content->setLayout(vl);
      auto grid = new QGridLayout;
//      grid->setSpacing(10);
      vl->addLayout(grid);
      vl->addStretch(1);
      ColorStackedLayout->addWidget(content);
      content->setProperty("parameter", paramKey);

      auto colorLabelCaption = new QLabel(ParameterToString(paramKey));
      colorLabelCaption->setStyleSheet(Style::Genesis::GetH4());
      auto colorComboCaption = new QLabel(tr("Color"));
      colorComboCaption->setStyleSheet(Style::Genesis::GetH4());

      AddRow(grid, colorLabelCaption, colorComboCaption);

      auto fillComboAndSet = [this](QComboBox* combo, GPShapeItem::DefaultColor color)
      {
        int ind = 0;
        for(auto& c : ColorsList)
        {
          if(c.second == color)
            ind = combo->count();
          combo->addItem(c.first, GPShapeItem::ToString(c.second), c.second);
        }
        combo->setCurrentIndex(ind);
      };
      for(auto val : ColorStyle[paramKey].keys())
      {
        auto color = ColorStyle[paramKey][val];
        auto combo = new QComboBox;
        fillComboAndSet(combo, color);
        connect(combo, &QComboBox::currentIndexChanged, this, [this, val, paramKey, combo](int ind)
        {
          ColorStyle[paramKey][val] = (GPShapeItem::DefaultColor)combo->itemData(ind).toInt();
        });
        AddRow(grid, val, combo);
      }
    }

    auto colorGroupParam = Plot->GetColorSortParam();
    ColorGroupCombo->addItem(tr("Don't group by colors"), ParameterType::LastType);
    int currentGroupInd = 0;
    for(int i = 0; i < ParameterType::LastType; i++)
    {
      auto param = (ParameterType)i;
      if(param == colorGroupParam)
        currentGroupInd = ColorGroupCombo->count();
      ColorGroupCombo->addItem(ParameterToString(param), param);
    }

    connect(ColorGroupCombo, &QComboBox::currentIndexChanged, this, [this](int ind)
    {
      auto param = (ParameterType)(ColorGroupCombo->itemData(ind).toInt());
      SetStackedLayoutIndex(ColorStackedLayout, param);
    });

    ColorGroupCombo->setCurrentIndex(currentGroupInd);
  }

  if (auto apply = ButtonBox->button(QDialogButtonBox::Ok))
  {
    apply->setDefault(true);

    //connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  body->setStyleSheet(Style::Genesis::GetUiStyle());
}

void WebDialogPCALegendSettings::SetupShapes()
{
  auto toIcon = [](GPShape::ShapeType shape, QSize size) -> QIcon
  {
    QPixmap pixmap(size);
    pixmap.fill(QColor(0,0,0,0));
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.save();
    p.scale(size.width(), size.height());
    p.drawPath(GPShape::GetPath(shape));
    p.restore();
    return QIcon(pixmap);
  };
  for(int i = 0; i < GPShape::LastShape; i++)
  {
    auto shape = (GPShape::ShapeType)i;
    ShapesList.append({toIcon(shape, ComboIconSize), shape});
  }
}

void WebDialogPCALegendSettings::SetupColors()
{
  auto toIcon = [](GPShapeItem::DefaultColor color, QSize size) -> QIcon
  {
    QPixmap pixmap(size);
    pixmap.fill(GPShapeItem::ToColor(color));
    return QIcon(pixmap);
  };
  for(int i = 0; i < GPShapeItem::DefaultColor::LastColor; i++)
  {
    auto color = (GPShapeItem::DefaultColor)i;
    ColorsList.append({toIcon(color, ComboIconSize), color});
  }
}

void WebDialogPCALegendSettings::AddRow(QGridLayout *l, QString labelText, QWidget *w)
{
  auto label = new QLabel(labelText);
  label->setStyleSheet("color: #002033");
  AddRow(l, label, w);
}

void WebDialogPCALegendSettings::AddRow(QGridLayout *l, QLabel* label, QWidget *w)
{
  int row = l->rowCount();
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  l->addWidget(label, row, 0);
  l->addWidget(w, row, 1);
}

void WebDialogPCALegendSettings::SetStackedLayoutIndex(QStackedLayout *l, ParameterType param)
{
  for(int i = 0; i < l->count(); i++)
  {
    auto w = l->widget(i);
    auto v = w->property("parameter");
    if(v.isValid())
    {
      if(v.toInt() == param)
      {
        l->setCurrentIndex(i);
        return;
      }
    }
  }
}

}//namespace Dialogs
