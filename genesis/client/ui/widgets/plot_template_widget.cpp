#include "plot_template_widget.h"
#include "ui_plot_template_widget.h"

#include "genesis_style/style.h"
#include "logic/notification.h"
#include "logic/service/service_locator.h"
#include "logic/service/igraphics_plot_axis_modificator.h"
#include "ui/plots/triplot/triplot.h"
#include "logic/enums.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfoList>
#include <QImageReader>
#include "genesis_style/style.h"
#include "logic/notification.h"
#include "logic/models/plot_template_model.h"
#include "logic/models/plot_template_items/text_item.h"
#include "logic/models/plot_template_items/line_item.h"
#include "graphicsplot/graphics_items/graphics_item_interactive_polyline.h"

namespace {
enum SizeConst{
  DefaultSize,
  OneToOne,
  TwoToOne,
  TreeToTwo,
  FourToTree,
  SixteenToNine,
  LastConst = SixteenToNine
};
QMap <int, std::pair<QString, double>> title =
{
  { OneToOne, { QObject::tr("1:1"), 1 } },
  { TwoToOne, { QObject::tr("2:1"), 2 } },
  { TreeToTwo, { QObject::tr("3:2"), 3/2.0 } },
  { FourToTree, { QObject::tr("4:3"), 4/3.0 } },
  { SixteenToNine, { QObject::tr("16:9"), 16/9.0 } },
};
}

PlotTemplateWidget::PlotTemplateWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlotTemplateWidget)
    , m_model(nullptr)
{
  ui->setupUi(this);

  connect(ui->pbAddTemplate, &QPushButton::clicked, this, &PlotTemplateWidget::accept);
  connect(ui->pbAddPen, &QPushButton::clicked, this, &PlotTemplateWidget::addPolyLine);
  connect(ui->pbAddText, &QPushButton::clicked, this, &PlotTemplateWidget::addText);
  connect(ui->pbAddLine, &QPushButton::clicked, this, &PlotTemplateWidget::addLine);
  connect(ui->pbAddImage, &QPushButton::clicked, this, &PlotTemplateWidget::addPixmap);
  connect(ui->pbAddCursor, &QPushButton::clicked, this, &PlotTemplateWidget::resetAddMode);
  connect(ui->pbCancel, &QPushButton::clicked, this, &PlotTemplateWidget::cancel);
  connect(ui->pbPreviousStep, &QPushButton::clicked, this, &PlotTemplateWidget::previousStep);

  ui->lCaption->setStyleSheet(Style::Genesis::GetH2());
  ui->pbPreviousStep->setProperty("secondary", true);
  ui->pbAddTemplate->setProperty("blue", true);
  ui->pbCancel->setProperty("secondary", true);

  ui->twItems->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->horizontalSplitter->setStretchFactor(0, 1);
  ui->horizontalSplitter->setStretchFactor(1, 0);
  ui->verticalSplitter->setStretchFactor(0, 1);
  ui->verticalSplitter->setStretchFactor(1, 0);
  initSizeCombobox();
  connect(ui->twItems, &QTreeWidget::customContextMenuRequested,this, &PlotTemplateWidget::showTreeWidgetMenu);
  connect(ui->horizontalSplitter, &QSplitter::splitterMoved, this, &PlotTemplateWidget::plotSizeChange);
  connect(ui->verticalSplitter, &QSplitter::splitterMoved, this, &PlotTemplateWidget::plotSizeChange);
  connect(ui->cbSize, &QComboBox::currentTextChanged, this, &PlotTemplateWidget::plotSizeSelect);
  ui->cbSize->setCurrentIndex(SixteenToNine);
}

PlotTemplateWidget::~PlotTemplateWidget()
{
  delete ui;
}

void PlotTemplateWidget::initSizeCombobox()
{
  ui->cbSize->setEditable(true);
  ui->cbSize->insertItem(DefaultSize, "");
  plotSizeChange();
  for (int i = OneToOne; i <= LastConst; i++)
  {
    ui->cbSize->insertItem(i, title[i].first, title[i].second);
  }
}

void PlotTemplateWidget::plotSizeSelect()
{
  QSize maximalSize = ui->verticalSplitter->size();
  QSize size = maximalSize;
  double coefficent = 1;
  if (ui->cbSize->currentIndex() != DefaultSize)
  {
    coefficent = ui->cbSize->currentData().toDouble();
  }
  else
  {
    QString text = ui->cbSize->currentText();
    coefficent = text.toDouble();
  }

  if (coefficent == 0)
    return;

  double resizeHeight = size.width() / coefficent;
  double resizeWidth = size.height() * coefficent;
  if (size.height() > resizeHeight)
  {
    size.setHeight(resizeHeight);
  }
  else
  {
    size.setWidth(resizeWidth);
  }
  if (m_model)
    m_model->getRootItem()->setSize(size);
}

void PlotTemplateWidget::plotSizeChange()
{
  QSize size = ui->wdtPlot->size();
  if (m_model)
    m_model->getRootItem()->setSize(size);
}

void PlotTemplateWidget::updatePlotSize()
{
  QSize size = m_model->getRootItem()->size();
  if (!size.isValid() || size.height() < 0 || size.width() < 0)
    return;

  QSize maximalSize = ui->verticalSplitter->size();
  double coefficent = (double)size.width() / size.height();
  if (maximalSize.height() < size.height())
  {
    size.setHeight(maximalSize.height());
    size.setWidth(maximalSize.height() * coefficent);
  }

  if (maximalSize.width() < size.width())
  {
    size.setWidth(maximalSize.width());
    size.setHeight(maximalSize.width() / coefficent);
  }

  int comboIndex = DefaultSize;

  for (int id = OneToOne; id <= LastConst; id++)
  {
    double coeff = title[id].second;
    if (abs(coefficent - coeff) <= 0.01)
    {
      comboIndex = id;
      break;
    }
  }

  ui->cbSize->blockSignals(true);
  if (comboIndex == DefaultSize)
  {
    ui->cbSize->setCurrentIndex(DefaultSize);
    ui->cbSize->setCurrentText(QString::number((double)size.width()/size.height(),'g', 2));
  }
  else
  {
    ui->cbSize->setCurrentIndex(comboIndex);
  }
  ui->cbSize->blockSignals(false);

  ui->horizontalSplitter->blockSignals(true);
  ui->horizontalSplitter->setSizes({size.width(), maximalSize.width() - size.width() - ui->horizontalSplitter->handleWidth()});
  ui->horizontalSplitter->blockSignals(false);
  ui->verticalSplitter->blockSignals(true);
  ui->verticalSplitter->setSizes({size.height(), maximalSize.height() - size.height() - ui->verticalSplitter->handleWidth()});
  ui->verticalSplitter->blockSignals(false);

  ui->wdtPlot->setToDefaultAxisX();
  ui->wdtPlot->setToDefaultAxisY();
}

void PlotTemplateWidget::showEvent(QShowEvent *event)
{
  updatePlotSize();
  QWidget::showEvent(event);
}

void PlotTemplateWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
}

void PlotTemplateWidget::setModel(PlotTemplateModel *model)
{
  if (model)
  {
    if (m_model)
    {
      disconnect(ui->pbClearPlot, &QPushButton::clicked, m_model, &PlotTemplateModel::removeAllItems);
      disconnect(ui->pbCancel, &QPushButton::clicked, m_model, &PlotTemplateModel::cancelChanges);
      disconnect(m_model->getRootItem(), &RootItem::sizeChanged, this, &PlotTemplateWidget::updatePlotSize);
    }
    m_model = model;
    ui->twItems->setModel(m_model);
    ui->wdtPlot->setModel(m_model);
    updatePlotSize();
    connect(ui->pbClearPlot, &QPushButton::clicked, m_model, &PlotTemplateModel::removeAllItems);
    connect(ui->pbCancel, &QPushButton::clicked, m_model, &PlotTemplateModel::cancelChanges);
    connect(m_model->getRootItem(), &RootItem::sizeChanged, this, &PlotTemplateWidget::updatePlotSize);
  }
}

void PlotTemplateWidget::setEditMode(bool isEdit)
{
  if (isEdit)
  {
    ui->lCaption->setText(tr("Editing plot template"));
    ui->pbAddTemplate->setText(tr("Save"));
  }
  else
  {
    ui->lCaption->setText(tr("Adding plot template"));
    ui->pbAddTemplate->setText(tr("Add template"));
  }
}

void PlotTemplateWidget::setPlotType(int type)
{
  const auto modificator = Service::ServiceLocator::Instance().Resolve<Triplot, Service::IGraphicsPlotAxisModificator>();
  if (type == static_cast<int>(TemplatePlot::PlotType::Triplot))
    modificator->ApplyModificator(ui->wdtPlot);
  else
    modificator->UndoModificator(ui->wdtPlot);
}

void PlotTemplateWidget::addPixmap()
{
  resetAddMode();
  auto files = QFileDialog::getOpenFileNames(this, tr("Select image"), "", tr("Image files")+" (*.png;*.jpg;*.bmp;*.pbm;*.pgm;*.ppm;*.xbm;*.xpm;*.svg)"+";;"+tr("All files")+" (*.*)");
  if (!files.isEmpty())
  {
    QImageReader reader(files.first());
    if (reader.canRead())
    {
      QPixmap pixmap = QPixmap::fromImage(reader.read());

      // calculate item rect for save aspect ratios
      QSize size = pixmap.size();
      QRect plotRect = ui->wdtPlot->axisRect()->rect();
      if (size.width() > size.height())
        size = size * (plotRect.width() / (2.0 * size.width()));
      else
        size = size * (plotRect.height() / (2.0 * size.height()));
      QRectF rect;
      rect.setSize(size);
      rect.moveTo(plotRect.center() - rect.center());
      QPointF topLeftCoord;
      topLeftCoord.setX(ui->wdtPlot->xAxis->pixelToCoord(rect.topLeft().x()));
      topLeftCoord.setY(ui->wdtPlot->yAxis->pixelToCoord(rect.topLeft().y()));
      QPointF bottomRightCoord;
      bottomRightCoord.setX(ui->wdtPlot->xAxis->pixelToCoord(rect.bottomRight().x()));
      bottomRightCoord.setY(ui->wdtPlot->yAxis->pixelToCoord(rect.bottomRight().y()));

      PixmapItem* pixmapItem = m_model->addPixmap();
      pixmapItem->setPixmap(pixmap);
      pixmapItem->setTopLeftCoordinate(topLeftCoord);
      pixmapItem->setTopLeftEditableCoordinate(topLeftCoord);
      pixmapItem->setBottomRightCoordinate(bottomRightCoord);
      pixmapItem->setBottomRightEditableCoordinate(bottomRightCoord);
    }
    else
    {
      Notification::NotifyError(tr("Error reading the image file"), tr("Error"));
    }
  }
}

void PlotTemplateWidget::addPolyLine()
{
  auto clickFunc = [this](QMouseEvent* event)
  {
    // auto polyline = new GraphicsItemInteractivePolyline(ui->wdtPlot);
    // polyline->setSelected(true);
    auto polyline = m_model->addPolyline();
    auto otherClickFunc = [this, polyline](QMouseEvent* event)
    {
      auto points = polyline->points();
      bool hasEnoughPoints = points.size() > 1;
      bool hitStartPoint = false;
      if(hasEnoughPoints)
      {
        auto clickPos = event->position();
        auto startPoint = points.first().toPointF();
        startPoint = ui->wdtPlot->coordToPixel(startPoint);
        auto selTolerance = ui->wdtPlot->selectionTolerance();
        hitStartPoint = (clickPos - startPoint).manhattanLength() < selTolerance;
      }

      if(hasEnoughPoints && hitStartPoint)
      {
        polyline->setLineClosed(true);
        ui->wdtPlot->resetClickFunc();
        return;
      }
      points.append(ui->wdtPlot->pixelToCoord(QPointF(event->pos())));
      polyline->setPoints(points);
    };
    auto points = polyline->points();
    points.append(ui->wdtPlot->pixelToCoord(QPointF(event->pos())));
    polyline->setPoints(points);
    ui->wdtPlot->setClickFunc(otherClickFunc);
  };

  resetAddMode();
  ui->wdtPlot->setClickFunc(clickFunc);
}

void PlotTemplateWidget::addText()
{
  resetAddMode();
  auto clickFunc = [this](QMouseEvent* event)
  {
    auto textItem = m_model->addText();
    QPointF coord = ui->wdtPlot->pixelToCoord(QPointF(event->pos()));
    textItem->setPosition(coord);
    textItem->setText(textItem->name());
    textItem->setSelected(true);
    ui->wdtPlot->resetClickFunc();
  };
  ui->wdtPlot->setClickFunc(clickFunc);
}

void PlotTemplateWidget::addLine()
{
  auto clickFunc = [this](QMouseEvent* event)
  {
    auto line = m_model->addLine();
    line->setSelected(true);
    auto otherClickFunc = [this, line](QMouseEvent* event)
    {
      auto clickPos = event->position();
      QPointF coord = ui->wdtPlot->pixelToCoord(QPointF(event->pos()));
      line->setEnd(coord);
      resetAddMode();
    };
    auto clickPos = event->position();
    QPointF coord = ui->wdtPlot->pixelToCoord(QPointF(event->pos()));
    line->setStart(coord);
    line->setEnd(coord);
    ui->wdtPlot->setClickFunc(otherClickFunc);
  };

  resetAddMode();
  ui->wdtPlot->setClickFunc(clickFunc);
}

void PlotTemplateWidget::resetAddMode()
{
  for (auto item: ui->wdtPlot->selectedItems())
  {
    item->setSelected(false);
  }
  ui->wdtPlot->resetClickFunc();
  ui->wdtPlot->replot();
}

void PlotTemplateWidget::showTreeWidgetMenu(const QPoint &pos)
{
  if (m_model)
  {
    QTreeWidgetItem *topItem = ui->twItems->currentItem();
    auto item = ui->twItems->getModelItem(topItem);
    if (item)
    {
      auto treeWidgetMenu = new QMenu(ui->twItems);
      std::map<QAction*, std::function<void(void)>> actions;
      m_model->FillContextMenu(item, *treeWidgetMenu, actions);
      if (!actions.empty())
      {
        connect(treeWidgetMenu, &QMenu::triggered, [actions](QAction* action)
                {
                  auto a = actions.find(action);
                  if (a != actions.end())
                  {
                    a->second();
                  }
                });
        treeWidgetMenu->popup(ui->twItems->mapToGlobal(pos));
      }
    }
  }
}

// unused function for debug
void PlotTemplateWidget::save()
{
  auto fileName = QFileDialog::getSaveFileName(nullptr, tr("Choose export path"), "TemplatePlot", QString("*.%1;;All files (*.*)").arg("json"));

  if (!fileName.isEmpty())
  {
    QJsonObject json = m_model->saveToJson();

    QFile file(fileName);
    QJsonDocument document;
    document.setObject(json);
    file.open(QFile::WriteOnly);
    file.write(document.toJson());
    file.close();
  }
}

// unused function for debug
void PlotTemplateWidget::load()
{
  auto fileName = QFileDialog::getOpenFileName(this, tr("Select json"), "(.json)");
  if (!fileName.isEmpty())
  {
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QJsonDocument document;
    QJsonParseError error;
    document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error == QJsonParseError::NoError)
      m_model->loadFromJson(document.object());
  }
}
