#include "web_dialog_pca_items_settings.h"
#include "ui_web_dialog_pca_items_settings.h"

#include <genesis_style/style.h>
#include <ui/plots/gp_items/sample_info_plate.h>
#include <ui/plots/gp_items/gpshape_with_label_item.h>

WebDialogPCAItemsSettings::WebDialogPCAItemsSettings(QWidget* parent, QList<GPShapeWithLabelItem*> items, CountsPlot* plot)
  : Dialogs::Templates::Dialog(parent, QDialogButtonBox::StandardButton::Apply | QDialogButtonBox::Cancel /*| QDialogButtonBox::StandardButton::Reset*/)
  , ui(new Ui::WebDialogPCAItemsSettings)
  , mContent(new QWidget(nullptr))
  , Items(items)
  , Plot(plot)
{
  SetupUi();
}

WebDialogPCAItemsSettings::~WebDialogPCAItemsSettings()
{
  delete ui;
}

void WebDialogPCAItemsSettings::Accept()
{
  CustomItemStyle style;

  if(ui->comboBox_size->currentIndex() == -1)
    style.size = QSizeF();
  else
    style.size = ui->comboBox_size->currentData().toSizeF();

  if(ui->comboBox_shape->currentIndex() == -1)
    style.shape = GPShape::LastShape;
  else
    style.shape = static_cast<GPShape::ShapeType>(ui->comboBox_shape->currentData().toInt());

  if(ui->comboBox_color->currentIndex() == -1)
    style.color = GPShapeItem::LastColor;
  else
    style.color = static_cast<GPShapeItem::DefaultColor>(ui->comboBox_color->currentData().toInt());


  CustomItemStyleMap itemStyle;
  for (auto& item : Items)
  {
    auto newItemStyle = style;
    if(newItemStyle.shape == GPShape::LastShape)
      newItemStyle.shape = item->GetShape();
    if(newItemStyle.color == GPShapeItem::LastColor)
      newItemStyle.color = (GPShapeItem::DefaultColor)item->getColor();
    if(newItemStyle.size == QSizeF())
      newItemStyle.size = item->GetSize();
    itemStyle[item->property("sample_id").toInt()] = newItemStyle;
  }

  Plot->SetCustomItemStyle(itemStyle);
  Plot->UpdateStyle();

  WebDialog::Accept();
}

void WebDialogPCAItemsSettings::SetupUi()
{
  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);




  Dialogs::Templates::Dialog::Settings s;
  if (Items.size() == 1)
  {
    s.dialogHeader = tr("Edit view 1 sample");
  }
  else
  {
    s.dialogHeader = tr("Edit view %1 samples").arg(Items.size());
  }
  applySettings(s);

  SetupShapes(ui->comboBox_shape->iconSize());
  SetupColors(ui->comboBox_size->iconSize());

  for (auto& pair : ShapesList)
    ui->comboBox_shape->addItem(pair.first, GPShape::ToString(pair.second), pair.second);

  for (auto& pair : ColorsList)
    ui->comboBox_color->addItem(pair.first, GPShapeItem::ToString(pair.second), pair.second);

  ui->comboBox_size->addItem(tr("Small"), GPShapeItem::ToDefaultSize(GPShapeItem::Small));
  ui->comboBox_size->addItem(tr("Medium"), GPShapeItem::ToDefaultSize(GPShapeItem::Medium));
  ui->comboBox_size->addItem(tr("Big"), GPShapeItem::ToDefaultSize(GPShapeItem::Big));
  ui->comboBox_size->addItem(tr("Very Big"), GPShapeItem::ToDefaultSize(GPShapeItem::VeryBig));

  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setDefault(true);
    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  if(Items.empty())
    return;
  bool colorSame = true;
  bool shapeSame = true;
  bool sizeSame = true;
  auto color = Items[0]->getColor();
  auto shape = Items[0]->GetShape();
  auto size = Items[0]->GetSize().toSize();
  for(auto& item : Items)
  {
    if(colorSame && item->getColor() != color)
    {
      colorSame = false;
    }
    if(shapeSame && item->GetShape() != shape)
    {
      shapeSame = false;
    }
    if(sizeSame && item->GetSize() != size)
    {
      sizeSame = false;
    }
    if(!colorSame && !shapeSame && !sizeSame)
      break;
  }

  if(colorSame)
    ui->comboBox_color->setCurrentIndex(ui->comboBox_color->findData((GPShapeItem::DefaultColor)color));
  else
    ui->comboBox_color->setCurrentIndex(-1);

  if(shapeSame)
    ui->comboBox_shape->setCurrentIndex(ui->comboBox_shape->findData(shape));
  else
    ui->comboBox_shape->setCurrentIndex(-1);

  if(sizeSame)
    ui->comboBox_size->setCurrentIndex(ui->comboBox_size->findData(size));
  else
    ui->comboBox_shape->setCurrentIndex(-1);
}

void WebDialogPCAItemsSettings::SetupShapes(const QSize& iconSize)
{
  auto toIcon = [](GPShape::ShapeType shape, QSize size) -> QIcon
  {
    QPixmap pixmap(size);
    pixmap.fill(QColor(0, 0, 0, 0));
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
  for (int i = 0; i < GPShape::LastShape; i++)
  {
    auto shape = (GPShape::ShapeType)i;
    ShapesList.append({ toIcon(shape, iconSize), shape });
  }
}

void WebDialogPCAItemsSettings::SetupColors(const QSize& iconSize)
{
  auto toIcon = [](GPShapeItem::DefaultColor color, QSize size) -> QIcon
  {
    QPixmap pixmap(size);
    pixmap.fill(GPShapeItem::ToColor(color));
    return QIcon(pixmap);
  };
  for (int i = 0; i < GPShapeItem::DefaultColor::LastColor; i++)
  {
    auto color = (GPShapeItem::DefaultColor)i;
    ColorsList.append({ toIcon(color, iconSize), color });
  }
}
