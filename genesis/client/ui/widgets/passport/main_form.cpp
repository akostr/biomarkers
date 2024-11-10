#include "main_form.h"
#include "ui_main_form.h"
#include "component_control.h"
#include <logic/models/passport_combo_models.h>
#include <QStyledItemDelegate>
#include <QStaticText>
#include <QPainter>
#include <QLineEdit>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QRandomGenerator>
#include <QJsonObject>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <ui/genesis_window.h>
#include <genesis_style/style.h>

enum UiDataRoles
{
  SampleDataRole = Qt::UserRole + 1,
  NameStaticTextRole,
  UIDRole,
  LoadingPrecentageRole,
  CompletionPrecentageRole,
  FileIdRole
};

class ChromaItemDelegate : public QStyledItemDelegate
{
public:
  ChromaItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent)
  {
    mFont.setFamily(Style::GetSASSValue("fontFaceNormal"));
    mFont.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toDouble() + 0.5);
  };
  ~ChromaItemDelegate(){};


private:
  struct itemCache
  {
    int loadingPrecentage;
    int completionPrecentage;
    QPixmap progressPixmap;
    QStaticText chromaNameText;
  };
  mutable QList<itemCache> mItemsCache;
  const int margin = 8;
  const int spacing = 6;
  QFont mFont;

private:
  QPair<QPixmap, QStaticText> updateItem(const QModelIndex &index) const
  {
    double screenPixmapSide = 20;
    double drawDpi = 2;
    double dpi2PixmapSide = screenPixmapSide * drawDpi;
    while(mItemsCache.size() < index.model()->rowCount())
    {
      auto pixmap = QPixmap(dpi2PixmapSide, dpi2PixmapSide);
      pixmap.setDevicePixelRatio(drawDpi);
      mItemsCache.append({0, 0, pixmap, QStaticText()});
      mItemsCache.last().progressPixmap.fill(QColor(0,0,0,0));
    }
    auto diff = mItemsCache.size() - index.model()->rowCount();
    if(diff > 0 )
      mItemsCache.remove(mItemsCache.size() - diff, diff);
    QString chromaName = index.data(Qt::DisplayRole).toString();
    int chromaCompletionPrecentage = index.data(CompletionPrecentageRole).toInt();
    int oldCompletionPrecentage = mItemsCache[index.row()].completionPrecentage;
    int chromaLoadingPrecentage = index.data(LoadingPrecentageRole).toInt();
    int oldLoadingPrecentage = mItemsCache[index.row()].loadingPrecentage;
    QPixmap* progressPixmap = &mItemsCache[index.row()].progressPixmap;
    //if old name text is empty, then it's first time
    bool drawAnyway = index.row() != 0 && mItemsCache[index.row()].chromaNameText.text().isEmpty();
    QStaticText* nameStaticText = &mItemsCache[index.row()].chromaNameText;
    if(nameStaticText->text() != chromaName)
    {
      nameStaticText->setTextFormat(Qt::TextFormat::PlainText);
      nameStaticText->setText(chromaName);
      nameStaticText->prepare(QTransform(), mFont);
    }

    auto formCompletionArc = [drawDpi](QPixmap* pixmap, int precentage, QPainter* painter)
    {
      pixmap->fill(QColor(0,0,0,0));
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(QPen(QColor(0, 32, 51, 20), 4));
      painter->setBrush(Qt::NoBrush);
      QRect r(1,1,pixmap->width() - 2, pixmap->height() - 2);
      painter->drawEllipse(r);
      QConicalGradient g(10,10,100);
      g.setColorAt(0, QColor(34, 195, 142));
      g.setColorAt(0.5, QColor(34, 195, 142));
      g.setColorAt(1, QColor(0, 120, 210));
      painter->setPen(QPen(QBrush(g), 4));
      painter->drawArc(r, 90 * 16, (-360.0 / 100.0 * (double)precentage) * 16);
      painter->restore();
    };

    if(oldLoadingPrecentage != chromaLoadingPrecentage || oldCompletionPrecentage != chromaCompletionPrecentage || drawAnyway)
    {
      if(chromaLoadingPrecentage < 0)
      {
        auto pixmap = QPixmap(":/resource/icons/icon_error@2x.png");
        double side = screenPixmapSide * pixmap.devicePixelRatioF();
        *progressPixmap = pixmap.scaled(QSizeF(side, side).toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      }
      else if(chromaLoadingPrecentage < 100)
      {
        progressPixmap->setDevicePixelRatio(1);
        QPainter p(progressPixmap);
        formCompletionArc(progressPixmap, chromaCompletionPrecentage, &p);
        if(index.row() != 0) //skip common item
        {
          QString loadingPrecentageText = QString("%1%").arg(chromaLoadingPrecentage);
          p.setFont(QFont("Segoe UI", 14, 600));
          auto textRect = p.fontMetrics().boundingRect(loadingPrecentageText);
          int y = progressPixmap->height()/2 - textRect.height()/2;
          int x = progressPixmap->width()/2 - textRect.width()/2;
          p.drawText(QRect(x, y, textRect.width(), textRect.height()), loadingPrecentageText);
        }
        progressPixmap->setDevicePixelRatio(drawDpi);
      }
      else if(chromaLoadingPrecentage >= 100 && chromaCompletionPrecentage < 100)
      {
        progressPixmap->setDevicePixelRatio(1);
        QPainter p(progressPixmap);
        formCompletionArc(progressPixmap, chromaCompletionPrecentage, &p);

        auto pixmap = QPixmap(":/resource/icons/icon_success_on@2x.png");
        double side = screenPixmapSide * pixmap.devicePixelRatioF();
        p.drawPixmap(progressPixmap->rect(), pixmap.scaled(QSizeF(side,side).toSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        progressPixmap->setDevicePixelRatio(drawDpi);
      }
      else if(chromaLoadingPrecentage >= 100 && chromaCompletionPrecentage >= 100)
      {
        auto pixmap = QPixmap(":/resource/icons/icon_upload_done@2x.png");
        double side = screenPixmapSide * pixmap.devicePixelRatioF();
        *progressPixmap = pixmap.scaled(QSizeF(side,side).toSize(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      }
    }
    return {*progressPixmap, *nameStaticText};
  }
  // QAbstractItemDelegate interface
public:
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
  {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);
    const QRect &rect(opt.rect);
    auto texts = updateItem(index);

    painter->setClipRect(option.rect);
    if(opt.state & QStyle::State_Selected)
    {
      painter->save();
      painter->setBrush(QColor(0, 32, 51, 13));
      painter->setPen(Qt::NoPen);
      painter->drawRect(rect);
      painter->setPen(QPen(QColor(0, 120, 210), 2));
      painter->drawLine(QPoint(1,rect.center().y() - 12), QPoint(1, rect.center().y() + 12));
      painter->restore();
    }
    painter->save();
    painter->translate(rect.topLeft());
    painter->drawPixmap(QPoint(margin, margin), texts.first);
    painter->setFont(mFont);
    QColor hexColor = Style::GetSASSColor("fontColorSecondary");
    painter->setPen(hexColor);
    painter->drawStaticText(margin + texts.first.width() + spacing, margin, texts.second);
    painter->restore();
  }

public:
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
  {
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    auto texts = updateItem(index);
    return (QSize(texts.first.width() + texts.second.size().width(), std::max(texts.first.height(), texts.second.size().toSize().height()))
            + QSize(margin + spacing + margin, margin + margin));
  }
};


PassportMainForm::PassportMainForm(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::PassportMainForm),
  mModelsManager(this)
{
  setStyleSheet(Style::ApplySASS("QLabel {color: @fontColorSecondary;}"));
  ui->setupUi(this);
  connect(&mModelsManager, &PassportModelsManager::modelsLoaded, this, &PassportMainForm::modelsLoaded);
  connect(&mModelsManager, &PassportModelsManager::modelsLoaded, this, [this]()
    {
      isModelsLoaded = true;
      SetSampleData(mInputDataCache);
      hideLoadingOverlay();
    });
  setupUi();
}

PassportMainForm::~PassportMainForm()
{
  delete ui;
}

void PassportMainForm::loadComboModels(bool isFragment)
{
  isModelsLoaded = false;
  showLoadingOverlay();
  mModelsManager.loadModels(isFragment ? PassportModelsManager::FFragmentType : PassportModelsManager::FChromaType);
}

void PassportMainForm::SetSampleData(const Passport::SampleData &data)
{
  mInputDataCache = data;
  if(!isModelsLoaded)
    return;
  ui->passportHeader->SetData(data);
  ui->recommendedWidget->SetData(data.pureGeo);
  ui->extendedWidget->SetData(data.pureGeo);
  ui->jointRecommended->SetData(data.jointGeo);
  ui->jointExtended->SetData(data.jointGeo);
  ui->layerControl->SetLayers(data.jointGeo.layers);
  ui->fieldComboBox->setCurrentIndex(ui->fieldComboBox->findText(data.mixedGeo.field));
  while(ui->componentsStackedWidget->count() > 0)
  {
    ui->componentsStackedWidget->removeWidget(ui->componentsStackedWidget->widget(0));
    mComponentsTabBar->removeTab(0);
  }

  for(int i = 0; i < data.mixedGeo.components.size(); i++)
    addComponent(data.mixedGeo.components[i]);

  ui->chemPass->setData(data.sampleChemical);
}

Passport::SampleData PassportMainForm::SampleData()
{
  if(!isModelsLoaded)
    return mInputDataCache;

  Passport::SampleData data;
  {//header
    auto d = ui->passportHeader->Data();
    data.fluidType = d.fluidType;
    data.sampleType = d.sampleType;
    data.comment = d.comment;
  }
  {//recommendedWidget
    auto d = ui->recommendedWidget->Data();
    data.pureGeo.field = d.field;
    data.pureGeo.fieldId = d.fieldId;
    data.pureGeo.wellCluster = d.wellCluster;
    data.pureGeo.wellClusterId = d.wellClusterId;
    data.pureGeo.well = d.well;
    data.pureGeo.wellId = d.wellId;
    data.pureGeo.depth = d.depth;
    data.pureGeo.depth_tvd = d.depth_tvd;
//    data.pureGeo.depthType = d.depthType;
    data.pureGeo.sampleDate = d.sampleDate;
    data.pureGeo.layer = d.layer;
    data.pureGeo.layerId = d.layerId;
  }
  {//extendedWidget
    auto d = ui->extendedWidget->Data();
    data.pureGeo.source = d.source;
    data.pureGeo.altitude = d.altitude;
    data.pureGeo.perforationInterval = d.perforationInterval;
    data.pureGeo.stratum = d.stratum;
    data.pureGeo.fluidType = d.fluidType;
    data.pureGeo.clusterPumpingStation = d.clusterPumpingStation;
    data.pureGeo.boosterPumpingStation = d.boosterPumpingStation;
    data.pureGeo.oilGasProductionStation = d.oilGasProductionStation;
  }
  {//jointRecommended
    auto d = ui->jointRecommended->Data();
    data.jointGeo.field = d.field;
    data.jointGeo.fieldId = d.fieldId;
    data.jointGeo.wellCluster = d.wellCluster;
    data.jointGeo.wellClusterId = d.wellClusterId;
    data.jointGeo.well = d.well;
    data.jointGeo.wellId = d.wellId;
    data.jointGeo.depth = d.depth;
    data.jointGeo.depth_tvd = d.depth_tvd;
//    data.jointGeo.depthType = d.depthType;
    data.jointGeo.sampleDate = d.sampleDate;
  }
  {//jointExtended
    auto d = ui->jointExtended->Data();
    data.jointGeo.source = d.source;
    data.jointGeo.altitude = d.altitude;
    data.jointGeo.perforationInterval = d.perforationInterval;
    data.jointGeo.stratum = d.stratum;
    data.jointGeo.fluidType = d.fluidType;
    data.jointGeo.clusterPumpingStation = d.clusterPumpingStation;
    data.jointGeo.boosterPumpingStation = d.boosterPumpingStation;
    data.jointGeo.oilGasProductionStation = d.oilGasProductionStation;
    data.jointGeo.layers = ui->layerControl->GetSelectedLayers();
  }
  {//mixed field
    data.mixedGeo.field = ui->fieldComboBox->currentText();
    data.mixedGeo.fieldId = ui->fieldComboBox->currentData(PassportModelsManager::idRole).toInt();
  }
  {//mixed components
    data.mixedGeo.components.clear();
    for(int i = 0; i < ui->componentsStackedWidget->count(); i++)
    {
      auto component = qobject_cast<PassportComponentControl*>(ui->componentsStackedWidget->widget(i));
      if(component)
        data.mixedGeo.components.append(component->Data());
    }
  }
  {//chemical passport
    data.sampleChemical = ui->chemPass->data();
  }
  return data;
}

void PassportMainForm::setChromatogrammsList(const QMap<QUuid, QString> &chromasList)
{
  mChromasModel->setData(mChromasModel->index(0,0), QVariant::fromValue(Passport::SampleData()), SampleDataRole);
  clearChromas();
  for(auto it = chromasList.constKeyValueBegin(); it != chromasList.constKeyValueEnd(); it++)
  {
    auto item = new QStandardItem(it->second);
    item->setData(QVariant::fromValue(Passport::SampleData()), SampleDataRole);
    item->setData(it->first, UIDRole);

    ///buttons placeholders
    auto refreshItem = new QStandardItem();
    auto removeItem = new QStandardItem();
    ///
    QList<QStandardItem*> row;
    for(int i = ColumnChroma; i < ColumnLast; i++)
    {
      switch((ColumnNum)i)
      {
      case ColumnChroma:
        row << item;
        break;
      case ColumnRemove:
        row << removeItem;
        break;
      case ColumnRefresh:
        row << refreshItem;
        break;
      default:
        break;
      }
    }
    mChromasModel->appendRow(row);
    auto lastRow = mChromasModel->rowCount() - 1;
    auto ind = mChromasModel->index(lastRow, ColumnChroma);
    mRowsMap[it->first] = ind;
    mChromasModel->setData(ind, ColumnChroma, LoadingPrecentageRole);
    mChromasModel->setData(ind, ColumnChroma, CompletionPrecentageRole);
  }
  setSingleUi(false);
  switchToCommon();
  recalcGeometry();
}

void PassportMainForm::setSingleChromatogramm(QString chromaName, int chromaId, Passport::SampleData data)
{
  mChromasModel->setData(mChromasModel->index(0,0), QVariant::fromValue(Passport::SampleData()), SampleDataRole);
  clearChromas();

  auto item = new QStandardItem(chromaName);
  item->setData(QVariant::fromValue(data), SampleDataRole);
  mChromasModel->appendRow(item);
  auto lastRow = mChromasModel->rowCount() - 1;
  auto ind = mChromasModel->index(lastRow, ColumnChroma);
  mRowsMap[QUuid::createUuid()] = ind;
  mChromasModel->setData(ind, chromaId, FileIdRole);
  mChromasModel->setData(ind, 100, LoadingPrecentageRole);
  mChromasModel->setData(ind, data.completionPrecentage(), CompletionPrecentageRole);
  ui->tableView->selectionModel()->setCurrentIndex(mChromasModel->index(1,ColumnChroma), QItemSelectionModel::SelectionFlag::ClearAndSelect);
  setSingleUi(true);
  recalcGeometry();
}

void PassportMainForm::setChromatogramms(const QList<ChromaData>& chromasData)
{
  if(chromasData.size() == 1)
  {
    const auto& ch = chromasData.first();
    setSingleChromatogramm(ch.name, ch.id, ch.data);
    return;
  }

  mChromasModel->setData(mChromasModel->index(0,0), QVariant::fromValue(Passport::SampleData()), SampleDataRole);
  clearChromas();

  for(const auto& ch : chromasData)
  {
    auto item = new QStandardItem(ch.name);
    item->setData(QVariant::fromValue(ch.data), SampleDataRole);
    item->setData(ch.id, FileIdRole);
    item->setData(100, LoadingPrecentageRole);
    item->setData(ch.data.completionPrecentage(), CompletionPrecentageRole);
    mChromasModel->appendRow(item);
    mRowsMap[QUuid::createUuid()] = QPersistentModelIndex(mChromasModel->index(mChromasModel->rowCount() - 1, 0));
  }

  //set view to common and load common sample data
  setSingleUi(false);
  switchToCommon();
  recalcGeometry();
}

void PassportMainForm::updateChromaLoadingPrecentage(QUuid uid, int loadingPrecentage)
{
  if(!mRowsMap.contains(uid))
    return;
  auto ind = mRowsMap[uid];
  mChromasModel->setData(ind, loadingPrecentage, LoadingPrecentageRole);
  if(loadingPrecentage < 0)
  {
    setupRemoveButton(ind.row());
    setupRefreshButton(ind.row());
  }
  else
  {
    removeRemoveButton(ind.row());//nothing happens if there was no remove button
    removeRefreshButton(ind.row());//nothing happens if there was no refresh button
    recalcGeometry();
  }
  if(loadingPrecentage == 100)
  {
    handleFilesUploading();
  }

}

void PassportMainForm::updateChromaCompletionPrecentage(QUuid uid, int completionPrecentage)
{
  if(!mRowsMap.contains(uid))
    return;
  auto ind = mRowsMap[uid];
  mChromasModel->setData(ind, completionPrecentage, CompletionPrecentageRole);
}

void PassportMainForm::updateChromaFileId(QUuid uid, int fileId)
{
  if(!mRowsMap.contains(uid))
    return;
  auto ind = mRowsMap[uid];
  mChromasModel->setData(ind, fileId, FileIdRole);
}

bool PassportMainForm::allChromasHasFileId()
{
  for(int r = 1; r < mChromasModel->rowCount(); r++) //row 0 is Common item
  {
    auto data = mChromasModel->data(mChromasModel->index(r, ColumnChroma), FileIdRole);
    if(!data.isValid() || data.isNull())
      return false;
  }
  return true;
}

QJsonArray PassportMainForm::toJson()
{
  mChromasModel->setData(ui->tableView->currentIndex(), QVariant::fromValue(SampleData()), SampleDataRole);
  QJsonArray root;
  //zero row is common item!!!
  for(int row = 1; row < mChromasModel->rowCount(); row++)
  {
    auto index = mChromasModel->index(row, ColumnChroma);
    auto fileId = mChromasModel->data(index, FileIdRole).toInt();
    if(fileId == -1)
      continue;
    QJsonObject jchroma;
    Passport::SampleData chroma = mChromasModel->data(index, SampleDataRole).value<Passport::SampleData>();
    jchroma["sample_title"] = mChromasModel->data(index).toString();
    jchroma["file_id"] = fileId;
    jchroma["sample_type"] = chroma.sampleType;
    if(chroma.fluidType != Passport::FEmpty)
    {
      jchroma["fluid_type"] = chroma.fluidType;
    }
    else
    {
      jchroma["fluid_type"] = 0;
    }

    jchroma["comment"] = chroma.comment;

    switch(chroma.sampleType)
    {
    case Passport::SPure:
    case Passport::SEmpty:
      jchroma["geological_passport"] = chroma.pureGeo.toJson((Passport::FluidType)chroma.fluidType);
      break;
    case Passport::SMixed:
      jchroma["geological_passport"] = chroma.mixedGeo.toJson((Passport::FluidType)chroma.fluidType);
      break;
    case Passport::SJoint:
      jchroma["geological_passport"] = chroma.jointGeo.toJson((Passport::FluidType)chroma.fluidType);
      break;
    }

    if(chroma.fluidType != Passport::FWater)
      jchroma["chemical_passport"] = chroma.sampleChemical.toJson((Passport::FluidType)chroma.fluidType);
    root << jchroma;
  }
  return root;
}

bool PassportMainForm::CheckValidity()
{
  for(int r = 1; r < mChromasModel->rowCount(); r++)
  {
    auto index = mChromasModel->index(r, ColumnChroma);
    Passport::SampleData sampleData ;

    bool needSwitch = true;
    if(ui->tableView->currentIndex().row() == index.row())//save current state
    {
      mChromasModel->setData(index, QVariant::fromValue(this->SampleData()), SampleDataRole);
      needSwitch = false;
    }

    sampleData = mChromasModel->data(index, SampleDataRole).value<Passport::SampleData>();

    if(!sampleData.isValid())
    {
      if(needSwitch)
        ui->tableView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);

      switch((Passport::SampleType)sampleData.sampleType)
      {
      case Passport::SMixed:
        for(int i = 0; i < ui->componentsStackedWidget->count(); i++)
        {
          auto w = qobject_cast<PassportComponentControl*>(ui->componentsStackedWidget->widget(i));
          if(!w->IsDataValid())
          {
            ui->componentsStackedWidget->setCurrentIndex(i);
            w->HighlightInvalid();
            return false;
          }
        }
        break;
      case Passport::SJoint:
        ui->layerControl->HighlightEmpty();
        return false;
      default:
        break;
      }
    }
  }
  return true;
}

void PassportMainForm::setupUi()
{
  ui->recommendedWidget->SetModelsManager(&mModelsManager);
  ui->jointRecommended->SetModelsManager(&mModelsManager);
  ui->extendedWidget->SetModelsManager(&mModelsManager);
  ui->jointExtended->SetModelsManager(&mModelsManager);
  ui->passportHeader->SetModelsManager(&mModelsManager);
  ui->chemPass->SetModelsManager(&mModelsManager);
  ui->layerControl->SetLayersComboModel(mModelsManager.getLayersModel());

  auto polish = [](QWidget* w){w->style()->polish(w);};

  ui->menuButton->setProperty("menu_secondary", true);
  polish(ui->menuButton);
  ui->clearButton->setProperty("secondary", true);
  polish(ui->clearButton);
  ui->captionLabel->setStyleSheet(Style::Genesis::Fonts::H2());
  polish(ui->captionLabel);

  QString headingButtonsStyle = Style::ApplySASS(
  "QPushButton"
  "{"
    "padding: 10px 10px 10px 14px;"
    "text-align: left;"
    "background-color:   @dialogBoxButtonSecondaryBg;"
    "border-color:       @dialogBoxButtonSecondaryBg;"
    "color:              @dialogBoxButtonSecondaryFg;"
  "}"
  "QPushButton:pressed "
  "{"
    "background-color:   @dialogBoxButtonSecondaryBg;"
    "border-color:       @dialogBoxButtonSecondaryBg;"
    "color:              @dialogBoxButtonSecondaryFg;"
  "}"
                                  );
  ui->recommendedToggleButton->setStyleSheet(headingButtonsStyle);
  polish(ui->recommendedToggleButton);
  ui->extendedToggleButton->setStyleSheet(headingButtonsStyle);
  polish(ui->extendedToggleButton);
  ui->jointRecommendedToggleButton->setStyleSheet(headingButtonsStyle);
  polish(ui->jointRecommendedToggleButton);
  ui->jointExtendedToggleButton->setStyleSheet(headingButtonsStyle);
  polish(ui->jointExtendedToggleButton);

  ui->tableView->setStyleSheet(Style::ApplySASS("QListView::item {font: @RegularTextSelectListFont;}"));
  polish(ui->tableView);
  polish(ui->passportsTabWidget);

  style()->polish(this);
  connect(ui->passportHeader, &PassportHeaderFormControl::SampleTypeChanged, this, [this](Passport::SampleType sampleType)
  {
    switch(sampleType)
    {
    case Passport::InvalidSampleType:
      ui->stackedWidget->setCurrentIndex(3);
      break;
    case Passport::SPure:
    case Passport::SEmpty:
      ui->stackedWidget->setCurrentIndex(0);
      break;
    case Passport::SMixed:
      ui->stackedWidget->setCurrentIndex(1);
      break;
    case Passport::SJoint:
      ui->stackedWidget->setCurrentIndex(2);
      break;
    }
  });

  connect(ui->passportHeader, &PassportHeaderFormControl::FluidChanged, this, [this](Passport::FluidType fluid)
  {
    ui->extendedWidget->onFluidChanged(fluid);
    ui->jointExtended->onFluidChanged(fluid);
    ui->chemPass->onFluidTypeChanged(fluid);
    switch(fluid)
    {
    case Passport::InvalidFluid:
      ui->passportsTabWidget->setTabEnabled(0, false);
    case Passport::FWater:
      ui->passportsTabWidget->setTabEnabled(1, false);
      break;
    case Passport::FGas:
    case Passport::FOil:
    case Passport::FCondensate:
    case Passport::FMixed:
    case Passport::FBitumoid:
    case Passport::FEmpty:
      ui->passportsTabWidget->setTabEnabled(0, true);
      ui->passportsTabWidget->setTabEnabled(1, true);
      break;
    }
  });
  ui->fieldComboBox->setModel(mModelsManager.getFieldsModel());
  ui->passportsTabWidget->tabBar()->setExpanding(true);
  auto style = ui->passportsTabWidget->style();
  style->unpolish(ui->passportsTabWidget);
  style->polish(ui->passportsTabWidget);
  ui->splitter->setStretchFactor(0, 0);
  ui->splitter->setStretchFactor(1, 1);

  mComponentsTabBar = new QTabBar();
  ui->componentsTabBarLayout->insertWidget(0, mComponentsTabBar);
  mComponentsTabBar->setStyleSheet("QTabBar::close-button {"
                                   "image: url(:/resource/icons/icon_dnd_area_remove.png);"
                                   "}");
  connect(mComponentsTabBar, &QTabBar::tabCloseRequested, this, [this](int ind)
  {
    ui->componentsStackedWidget->removeWidget(ui->componentsStackedWidget->widget(ind));
    mComponentsTabBar->removeTab(ind);
    if(mComponentsTabBar->count() <= 2)
      mComponentsTabBar->setTabsClosable(false);
    recalcSummarySubstance();
  });
  connect(ui->addComponentTabButton, &QPushButton::clicked, this, [this](){this->addComponent();});
  connect(mComponentsTabBar, &QTabBar::currentChanged, ui->componentsStackedWidget, [this](int newCurrent)
  {
    QSignalBlocker sb(ui->componentsStackedWidget);
    ui->componentsStackedWidget->setCurrentIndex(newCurrent);
  });
  connect(ui->componentsStackedWidget, &QStackedWidget::currentChanged, mComponentsTabBar, [this](int newCurrent)
  {
    QSignalBlocker sb(mComponentsTabBar);
    mComponentsTabBar->setCurrentIndex(newCurrent);
  });
  Passport::SampleHeaderData data;
  data.sampleType = Passport::SPure;
  data.fluidType = Passport::FGas;
  ui->passportHeader->SetData(data);

  mCommonMenu = new QMenu(ui->menuButton);
  mItemMenu = new QMenu(ui->menuButton);
  auto applyToAllAction = new QAction(tr("Apply to all"), ui->menuButton);
  mCommonMenu->addAction(applyToAllAction);
  auto copyAction = new QAction(tr("Copy"), ui->menuButton);
  auto pasteAction = new QAction(tr("Paste"), ui->menuButton);
  mCommonMenu->addAction(copyAction);
  mCommonMenu->addAction(pasteAction);
  mItemMenu->addAction(copyAction);
  mItemMenu->addAction(pasteAction);

  connect(applyToAllAction, &QAction::triggered, this, [this]()
  {
    mChromasModel->setData(mChromasModel->index(0, ColumnChroma), QVariant::fromValue(SampleData()), SampleDataRole);
    for(int row = 1; row < mChromasModel->rowCount(); row++)
      mChromasModel->setData(mChromasModel->index(row, ColumnChroma), mCommonItem->data(SampleDataRole), SampleDataRole);
  });

  connect(copyAction, &QAction::triggered, this, [this]()
  {
    mDataBuffer = SampleData();
  });
  connect(pasteAction, &QAction::triggered, this, [this]()
  {
    SetSampleData(mDataBuffer);
  });

  mChromasModel = new QStandardItemModel(0, 3, ui->tableView);
  ui->tableView->setItemDelegateForColumn(ColumnChroma, new ChromaItemDelegate(ui->tableView));
  ui->tableView->setModel(mChromasModel);
  ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
  ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
  ui->tableView->horizontalHeader()->setMinimumSectionSize(5);

  ui->tableView->horizontalHeader()->hide();
  ui->tableView->verticalHeader()->hide();
  ui->tableView->setShowGrid(false);
  ui->tableView->setStyleSheet("QTableView::item {border: none; padding: 0px; margin: 0px;}");
  ui->tableView->viewport()->installEventFilter(this);
  ui->tableView->installEventFilter(this);
  connect(ui->tableView, &QAbstractItemView::clicked, this, [](const QModelIndex& ind)
          {
//            qDebug() << "clicked on index" << ind;
          });
  auto sm = ui->tableView->selectionModel();
  connect(sm, &QItemSelectionModel::currentChanged, this, [this, sm](const QModelIndex& current, const QModelIndex& previous)
  {
//    qDebug() << previous << current;
    auto matchedPrevious = mChromasModel->index(previous.row(), ColumnChroma);
    auto matchedCurrent = mChromasModel->index(current.row(), ColumnChroma);
    if(matchedCurrent.row() == matchedPrevious.row())
      return;
//    qDebug() << "load models";
    if(previous.isValid())
    {
      auto data = this->SampleData();
      mChromasModel->setData(matchedPrevious, QVariant::fromValue(data), SampleDataRole);
      mChromasModel->setData(matchedPrevious, data.completionPrecentage(), CompletionPrecentageRole);
    }
    if(!matchedCurrent.isValid())
    {
      sm->setCurrentIndex(mChromasModel->indexFromItem(mCommonItem), QItemSelectionModel::ClearAndSelect);
      return;
    }
    SetSampleData(matchedCurrent.data(SampleDataRole).value<Passport::SampleData>());
    if(mChromasModel->itemFromIndex(matchedCurrent) == mCommonItem)
      ui->menuButton->setMenu(mCommonMenu);
    else
      ui->menuButton->setMenu(mItemMenu);
    ui->captionLabel->setText(matchedCurrent.data().toString());
  });
  mCommonItem = new QStandardItem(tr("Template"));
//  mCommonItem->setData(QVariant::fromValue(SampleData()), SampleDataRole);
  mCommonItem->setData(100, LoadingPrecentageRole);
  mChromasModel->appendRow(mCommonItem);
  mChromasModel->setData(mChromasModel->indexFromItem(mCommonItem), QVariant::fromValue(Passport::SampleData()), SampleDataRole);
  mDataBuffer = Passport::SampleData();

////DEBUG
//  QStringList chromas;
//  for(int i = 0; i < 10; i++)
//    chromas << QString("chroma %1").arg(i);
//  setChromatogrammsList(chromas);

//  auto timer = new QTimer(this);
//  timer->setInterval(500);
//  auto  debug_precentage = new QMap<QString, int>();
//  for(auto& c : chromas) (*debug_precentage)[c] = 0;
//  connect(timer, &QTimer::timeout, this, [chromas, debug_precentage, this, timer]()
//  {
//    for(auto& chroma : chromas)
//    {
//      (*debug_precentage)[chroma] += QRandomGenerator::global()->bounded(5);
//      updateChromaPrecentage(chroma, (*debug_precentage)[chroma]);
//    }
//    bool end = true;
//    for(auto& p : *debug_precentage)
//      if(p < 100) end = false;
//    if(end)
//    {
//      timer->stop();
//      timer->deleteLater();
//      delete debug_precentage;
//    }
//  });
//  timer->start();
////DEBUG
  recalcTabOrder();

  for(auto& c : findChildren<QComboBox*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : findChildren<QAbstractSpinBox*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : findChildren<QTabBar*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : findChildren<QLineEdit*>())
  {
    c->installEventFilter(this);
  }
}

void PassportMainForm::addComponent(const Passport::MixedSampleGeoComponent &component)
{
  int ind = 1;
  if(mComponentsTabBar->count() > 0)
  {
    ind = mComponentsTabBar->tabData(mComponentsTabBar->count() - 1).toInt() + 1;
  }
  mComponentsTabBar->addTab(QString("%1 %2").arg(tr("Component")).arg(ind));
  mComponentsTabBar->setTabData(mComponentsTabBar->count() - 1, ind);
  auto newComponent = new PassportComponentControl();
  newComponent->setModelsManager(&mModelsManager);
  connect(ui->passportHeader, &PassportHeaderFormControl::FluidChanged, newComponent, &PassportComponentControl::onFluidChanged);
  newComponent->onFluidChanged((Passport::FluidType)ui->passportHeader->Data().fluidType);
  connect(ui->fieldComboBox, &QComboBox::currentIndexChanged, newComponent, [this, newComponent](int ind)
  {
    if(ind == -1)
      return;
    newComponent->onFieldChanged(ui->fieldComboBox->itemData(ind, PassportModelsManager::idRole).toInt(),
                                 ui->fieldComboBox->itemData(ind, PassportModelsManager::childModelRole).value<QStandardItemModel*>());
  });
  if(ui->fieldComboBox->currentIndex() != -1)
  {
    auto ind = ui->fieldComboBox->currentIndex();
    newComponent->onFieldChanged(ui->fieldComboBox->itemData(ind, PassportModelsManager::idRole).toInt(),
                                 ui->fieldComboBox->itemData(ind, PassportModelsManager::childModelRole).value<QStandardItemModel*>());
  }
  newComponent->SetData(component);
  ui->componentsStackedWidget->addWidget(newComponent);
  if(mComponentsTabBar->count() > 2)
    mComponentsTabBar->setTabsClosable(true);

  connect(newComponent, &PassportComponentControl::substanceChanged, this, &PassportMainForm::recalcSummarySubstance);

  for(auto& c : newComponent->findChildren<QComboBox*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : newComponent->findChildren<QAbstractSpinBox*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : newComponent->findChildren<QTabBar*>())
  {
    c->setFocusPolicy(Qt::StrongFocus);
    c->installEventFilter(this);
  }
  for(auto& c : newComponent->findChildren<QLineEdit*>())
  {
    c->installEventFilter(this);
  }
  recalcSummarySubstance();
}

void PassportMainForm::recalcTabOrder()
{
  auto setTabOrder = [](QWidget* container, const QWidgetList& wlist)
  {
    for(int i = 0; i < wlist.size(); i++)
    {
      auto w = wlist[i];
      QWidget* prev = nullptr;
      if(i != 0) prev = wlist[i-1];
      container->setTabOrder(prev, w);
    }
  };

  {//main order
    QWidgetList mainOrderList {ui->menuButton, ui->clearButton};
    mainOrderList << ui->passportHeader->tabOrderWidgets();
    mainOrderList << ui->passportsTabWidget;
    setTabOrder(this, mainOrderList);
  }
  {//pure order
    QWidgetList pureOrderList {ui->recommendedToggleButton};
    pureOrderList << ui->recommendedWidget->tabOrderWidgets();
    pureOrderList << ui->extendedToggleButton;
    pureOrderList << ui->extendedWidget->tabOrderWidgets();
    setTabOrder(ui->pureProbePage, pureOrderList);
  }
  {//mixed order
    QWidgetList mixedOrderList {ui->fieldComboBox, mComponentsTabBar, ui->addComponentTabButton, ui->componentsStackedWidget};
    setTabOrder(ui->mixedProbePage, mixedOrderList);
  }
  auto calcJointOrderList = [this, setTabOrder]()
  {//joint order
    QWidgetList jointOrderList;
    jointOrderList << ui->layerControl->tabOrderWidgets();
    jointOrderList << ui->jointRecommendedToggleButton;
    jointOrderList << ui->jointRecommended->tabOrderWidgets();
    jointOrderList << ui->jointExtendedToggleButton;
    jointOrderList << ui->jointExtended->tabOrderWidgets();
    setTabOrder(ui->jointProbePage, jointOrderList);
  };
  connect(ui->layerControl, &iTabOrderWidget::tabOrderChanged, this, calcJointOrderList);
  calcJointOrderList();
}

void PassportMainForm::clearChromas()
{
  auto commonItem = mChromasModel->takeItem(0);
  mChromasModel->clear();
  commonItem->setData(QVariant::fromValue(Passport::SampleData()), SampleDataRole);
  mChromasModel->appendRow(commonItem);
  mRowsMap.clear();
}

void PassportMainForm::switchToCommon()
{
  ui->tableView->selectionModel()->setCurrentIndex(mChromasModel->index(0,ColumnChroma), QItemSelectionModel::SelectionFlag::ClearAndSelect);
}

void PassportMainForm::setupRemoveButton(int row)
{
  auto removeBtn = new QPushButton();
  auto removePixmap = QPixmap(":/resource/icons/icon_action_cross.png");
  auto removeIcon = QIcon(removePixmap);
  removeBtn->setAutoFillBackground(true);
  removeBtn->setIcon(removeIcon);
  removeBtn->setFlat(true);
  removeBtn->setStyleSheet("QPushButton {border: none; margin: 0px; padding: 0px; background: transparent;}");
  removeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto size = removePixmap.size() / removePixmap.devicePixelRatio();
  size.setHeight(ui->tableView->rowHeight(0));
  removeBtn->setMaximumSize(size);
  removeBtn->setMinimumSize(size);
  auto removeButtonInd = mChromasModel->index(row, ColumnRemove);
  ui->tableView->setIndexWidget(removeButtonInd, removeBtn);
  auto persistendRemoveInd = QPersistentModelIndex(removeButtonInd);
  connect(removeBtn, &QPushButton::clicked, this, [this, persistendRemoveInd]()
          {
            this->remove(persistendRemoveInd);
          });
  removeBtn->removeEventFilter(ui->tableView);
  recalcGeometry();
}

void PassportMainForm::setupRefreshButton(int row)
{
  auto refreshBtn = new QPushButton();
  auto refreshPixmap = QPixmap(":/resource/icons/icon_action_refresh.png");
  auto refreshIcon = QIcon(refreshPixmap);
  refreshBtn->setAutoFillBackground(true);
  refreshBtn->setIcon(refreshIcon);
  refreshBtn->setFlat(true);
  refreshBtn->setStyleSheet("QPushButton {border: none; margin: 0px; padding: 0px; background: transparent;}");
  refreshBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  auto size = refreshPixmap.size() / refreshPixmap.devicePixelRatio();
  size.setHeight(ui->tableView->rowHeight(0));
  refreshBtn->setMaximumSize(size);
  refreshBtn->setMinimumSize(size);
  auto refreshButtonInd = mChromasModel->index(row, ColumnRefresh);
  ui->tableView->setIndexWidget(refreshButtonInd, refreshBtn);
  auto persistendRefreshInd = QPersistentModelIndex(refreshButtonInd);
  connect(refreshBtn, &QPushButton::clicked, this, [this, persistendRefreshInd]()
          {
            this->refresh(persistendRefreshInd);
          });
  refreshBtn->removeEventFilter(ui->tableView);
  recalcGeometry();
}

void PassportMainForm::removeRefreshButton(int row)
{
  auto ind = mChromasModel->index(row, ColumnRefresh);
  ui->tableView->setIndexWidget(ind, nullptr);
}

void PassportMainForm::removeRemoveButton(int row)
{
  auto ind = mChromasModel->index(row, ColumnRemove);
  ui->tableView->setIndexWidget(ind, nullptr);
}

void PassportMainForm::handleFilesUploading()
{
  bool allFinished = true;
  bool withErrors = false;
  for(int r = 1; r < mChromasModel->rowCount(); r++) //row 0 is Common item
  {
    int prc = mChromasModel->index(r, 0).data(LoadingPrecentageRole).toInt();
    if(prc >= 0 && prc < 100)
    {
      allFinished = false;
      break;
    }
    if(prc < 0 && !withErrors)
      withErrors = true;
  }
  if(allFinished)
    emit allFilesUploadFinished(withErrors);
}

void PassportMainForm::showLoadingOverlay()
{
  if(!mLoadingOvelray)
  {
    mLoadingOvelray = new QWidget(this);
    mLoadingOvelray->setObjectName("removable_overlay");
    mLoadingOvelray->autoFillBackground();
    auto l = new QGridLayout(mLoadingOvelray);
    {
      auto hs = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum);
      auto h1s = new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum);
      auto vs = new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      auto v1s = new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding);
      l->addItem(vs, 0, 1);
      l->addItem(v1s, 2, 1);
      l->addItem(hs, 1, 0);
      l->addItem(h1s, 1, 2);
      l->addWidget(new QLabel(tr("Loading"), mLoadingOvelray), 1, 1);
    }
    mLoadingOvelray->setStyleSheet(".QWidget {background-color: #d9002033;}");
    mLoadingOvelray->setGeometry(rect());
  }
  mLoadingOvelray->show();
}

void PassportMainForm::hideLoadingOverlay()
{
  mLoadingOvelray->hide();
}

bool PassportMainForm::eventFilter(QObject *watched, QEvent *event)
{
  if(watched == ui->tableView->viewport())
  {
    //preventing changing selection on table button click
    if(event->type() == QEvent::Type::MouseButtonPress)
    {
      auto mouseEvent = (QMouseEvent*)event;
      auto ind = ui->tableView->indexAt(mouseEvent->pos());
      if(ind.column() == ColumnChroma)
        return false;
      else if(ind.column() == ColumnRefresh || ind.column() == ColumnRemove)
        return true;
    }
  }
  else if(watched == ui->tableView)
  {
    if(event->type() == QEvent::Type::FocusOut)
    {
      //exact case: QApplication::focusWidget() == one of our button
      //but anyway
      return true;
    }
  }
  else
  {
    //this event filter is only for controls to provide weak wheel behaviour for QAbstractSpinBox, QTabBar and QComboBox children
    if(!watched->isWidgetType())
      return false;
    auto widget = qobject_cast<QWidget*>(watched);
    if(event->type() == QEvent::Type::Wheel)
    {
      if(widget->hasFocus())
        return false;
      event->ignore();
      return true;
    }
    else if(event->type() == QEvent::Type::FocusOut)
    {
      auto combo = qobject_cast<QComboBox*>(watched);
      if(combo && combo->isEditable())
      {
        if(!combo->currentText().isEmpty() && combo->findText(combo->currentText()) == -1)
        {
          emit combo->lineEdit()->returnPressed();
        }
        else if(combo->currentText().isEmpty())
        {
          combo->setCurrentIndex(-1);
        }
      }
      auto matchedCurrent = mChromasModel->index(ui->tableView->currentIndex().row(), ColumnChroma);
      mChromasModel->setData(matchedCurrent, SampleData().completionPrecentage(), CompletionPrecentageRole);
    }
  }
  return false;
}

void PassportMainForm::on_recommendedToggleButton_toggled(bool checked)
{
  ui->recommendedWidget->setVisible(checked);
  ui->recommendedToggleButton->setIcon(checked ? (QIcon(":/resource/controls/collpse_up.png")) : (QIcon(":/resource/controls/collpse_dn.png")));
  ui->geoPass->updateGeometry();
}


void PassportMainForm::on_extendedToggleButton_toggled(bool checked)
{
  ui->extendedWidget->setVisible(checked);
  ui->extendedToggleButton->setIcon(checked ? (QIcon(":/resource/controls/collpse_up.png")) : (QIcon(":/resource/controls/collpse_dn.png")));
  ui->geoPass->updateGeometry();
}


void PassportMainForm::on_jointRecommendedToggleButton_toggled(bool checked)
{
  ui->jointRecommended->setVisible(checked);
  ui->jointRecommendedToggleButton->setIcon(checked ? (QIcon(":/resource/controls/collpse_up.png")) : (QIcon(":/resource/controls/collpse_dn.png")));
  ui->geoPass->updateGeometry();
}


void PassportMainForm::on_jointExtendedToggleButton_toggled(bool checked)
{
  ui->jointExtended->setVisible(checked);
  ui->jointExtendedToggleButton->setIcon(checked ? (QIcon(":/resource/controls/collpse_up.png")) : (QIcon(":/resource/controls/collpse_dn.png")));
  ui->geoPass->updateGeometry();
}

void PassportMainForm::on_clearButton_clicked()
{
  SetSampleData(Passport::SampleData());
}

void PassportMainForm::recalcSummarySubstance()
{
  QList<PassportComponentControl*> components;
  double sum = 0;
  for(int i = 0; i < ui->componentsStackedWidget->count(); i++)
  {
    components << qobject_cast<PassportComponentControl*>(ui->componentsStackedWidget->widget(i));
    if(auto last = components.last())
      sum += last->Substance();
  }
  for(auto& component : components)
    if(component)
      component->SetSummarySubstance(sum);
}

void PassportMainForm::setSingleUi(bool isSingle)
{
  ui->tableView->setVisible(!isSingle);
  ui->menuButton->setVisible(!isSingle);
}

void PassportMainForm::recalcGeometry()
{
  auto t = ui->tableView;
  auto vh = t->verticalHeader();
  for(int r = 0; r < mChromasModel->rowCount(); r++)
  {
    vh->resizeSection(r, t->rowHeight(r));
  }
  auto hh = t->horizontalHeader();
  for(int c = 0; c < mChromasModel->columnCount(); c++)
  {
    if(mChromasModel->rowCount() > 1)
    {
      QWidget* w = nullptr;
      for(int r = 0; r < mChromasModel->rowCount(); r++)
      {
        w = t->indexWidget(mChromasModel->index(r,c));
        if(w)
          break;
      }
      if(w)
        hh->resizeSection(c, w->width());
      else
        t->resizeColumnToContents(c);
    }
    else
    {
      t->resizeColumnToContents(c);
    }
  }
}

void PassportMainForm::remove(QPersistentModelIndex ind)
{
  mChromasModel->removeRow(ind.row());
  recalcGeometry();
  handleFilesUploading();
}

void PassportMainForm::refresh(QPersistentModelIndex ind)
{
  removeRefreshButton(ind.row());
  removeRemoveButton(ind.row());
  recalcGeometry();
  emit reloadChroma(mChromasModel->index(ind.row(), ColumnChroma).data(UIDRole).toUuid());
}


void PassportMainForm::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  if(mLoadingOvelray)
    mLoadingOvelray->setGeometry(rect());
}
