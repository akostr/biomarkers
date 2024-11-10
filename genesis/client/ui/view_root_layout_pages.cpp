#include "view_root_layout_pages.h"

#include <genesis_style/style.h>
#include <QSet>

#include <logic/known_context_tag_names.h>
#include "known_view_names.h"
#include <logic/context_root.h>
#include <ui/controls/circle_button.h>
#include <ui/genesis_window.h>
#include <QResizeEvent>
#include <QPainter>

using namespace Names;
using namespace ViewPageNames;
using namespace Core;

namespace
{
  int MinumunLenght = 60;
}
////////////////////////////////////////////////////
//// Root Layout Pages view class
namespace Views
{
ViewRootLayoutPages::ViewRootLayoutPages(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewRootLayoutPages::~ViewRootLayoutPages()
{
}

void ViewRootLayoutPages::SetupUi()
{
  //// Root layout
  RootLayout = new QHBoxLayout(this);
  RootLayout->setContentsMargins(0, 0, 0, 0);
  RootLayout->setSpacing(0);

  RootSplitter = new QSplitter(this);
  //RootSplitter->setStyleSheet(Style::GetStyleSheet(":/resource/styles/genesis_ui_qsplitter.qss"));
  RootLayout->addWidget(RootSplitter, 1);
  CollapseButton = new CircleButton(this);
  CollapseButton->setCheckable(true);
  connect(CollapseButton, &CircleButton::toggled, this, [this](bool isChecked)
          {
            auto sizes = RootSplitter->sizes();
            if(!isChecked)
              sizes[0] = RootSplitter->widget(0)->sizeHint().width();
            else
              sizes[0] = 0;
            RootSplitter->setSizes(sizes);
          });
  connect(GenesisWindow::Get(), &GenesisWindow::windowMouseMove, this, &ViewRootLayoutPages::UpdateCirclePosition);
  //// Menu area
  {
    Menu = View::Create(ViewRootLayoutPagesMenuPageName, this);
    Menu->installEventFilter(this);
    RootSplitter->addWidget(Menu);
  }

  //// Pages area
  {
    PagesArea = new QWidget(this);
    RootSplitter->addWidget(PagesArea);
    setCollapseButtonYShift(100);

    RootSplitter->setStretchFactor(0,0);
    RootSplitter->setStretchFactor(1,1);
    RootSplitter->handle(1)->setAttribute(Qt::WA_Hover);

    PagesArea->setContentsMargins(0, 0, 0, 0);
    PagesArea->setStyleSheet(Style::ApplySASS("background: @windowColor;"));

    PagesAreaLayout = new QVBoxLayout(PagesArea);
    PagesAreaLayout->setContentsMargins(0, 0, 0, 0);
    PagesAreaLayout->setSpacing(0);

    //// Content
    {
      //      //// Toolbar
      //      {
      //        Toolbar = View::Create(ViewRootToolbarPageName, PagesArea);
      //        PagesAreaLayout->addWidget(Toolbar);
      //      }

      //// Hline
      {
        auto hline = new QFrame(PagesArea);
        hline->setFrameShape(QFrame::HLine);
        hline->setObjectName("hline");
        hline->setStyleSheet(Style::Genesis::GetUiStyle());
        PagesAreaLayout->addWidget(hline);
      }

      //// Pages
      {
        PagesContainer = new QStackedWidget(PagesArea);
        PagesAreaLayout->addWidget(PagesContainer);
      }
    }
  }
}

QPointer<View> ViewRootLayoutPages::GetCurrentPage()
{
  return qobject_cast<View*>(PagesContainer->currentWidget());
}

QString ViewRootLayoutPages::GetCurrentPageId() const
{
  if(!PagesContainer->currentWidget())
    return QString();
  return PagesContainer->currentWidget()->property("page_id").toString();
}

void ViewRootLayoutPages::ApplyContextUi(const QString& dataId, const QVariant& data)
{
  QSet<QString> handledChildFactories = {
    ViewProjectInfoPageName,
    ViewProjectChildrenPageName,
    ViewProjectImportedFilesPageName,
    ViewChromatogrammsFragmentsPageName,
    ViewProjectMarkupPageName,
    ViewLibrary,
    ViewProjectModelsListPageName,
    ViewPageProjectPCAGraphicsPlotPageName,
    ViewPagePcaModelName,
    ViewProjectDataPageName,
    ViewProjectLogPageName,
    ViewNumericData,
    ViewPageProjectMarkupName,
    ViewPageAnalysisDataTablePageName,
    ViewProjectsPageName,
    ViewPageProfileName,
    ViewPageStatisticName,
    ViewIdentificationPageName,
    ViewInterpretationPageName,
    ViewLoginEditPageName,
    ViewPageProjectPLSGraphicsPlotPageName,
    ViewPagePlsModelPageName,
    ViewPageMcrModelPageName,
    ViewPageProjectPlsPredictName,
    ViewPageProjectMcrGraphicPlotPageName,
    ViewPageProjectMarkupVersionPageName,
    ViewPageHeightRatioMatrixPageName,
    ViewPageCreateIdentificationPlotName,
    ViewPageIdentificationPlotListName
  };

  bool isReset = isDataReset(dataId, data);
  QString pageId;
  if(isReset)
  {
    for(auto& page : Pages)
      page->deleteLater();
    Pages.clear();
    qInfo() << "clear pages";
    pageId = View::CurrentPageId();
  }
  else if(dataId != Names::ContextTagNames::Page)
  {
    return;
  }
  else
  {
    pageId = data.toString();
  }

  if(pageId.isEmpty())
    return;

  //// This view shall handle pages lazily
  //// Is requestd page supported by this view?
  if (handledChildFactories.contains(pageId))
  {
    //// Create if necessary
    if (!Pages.value(pageId))
    {
      Pages[pageId] = View::Create(pageId, PagesContainer);
      PagesContainer->addWidget(Pages[pageId]);
      connect(Pages[pageId], &View::logOutAcceptStatus, this, [this](bool isRejected)
              {
                if(!isRejected)
                {
                  View::SwitchToModule(ModulesContextTags::MNoModule);
                  View::SwitchToPage(ViewPageNames::ViewProjectsPageName);
                  GenesisContextRoot::Get()->GetContextUser()->SetData(Names::UserContextTags::kUserState, Names::UserContextTags::notAuthorized);
                }
              });
    }

    //// Created, use
    if (Pages.value(pageId))
      PagesContainer->setCurrentWidget(Pages[pageId]);

    Pages[pageId]->setProperty("page_id", pageId);
    qInfo() << "current page name: " << pageId;
  }
}

int ViewRootLayoutPages::getCollapseButtonYShift() const
{
  return CollapseButtonYShift;
}

void ViewRootLayoutPages::setCollapseButtonYShift(int newCollapseButtonYShift)
{
  if (CollapseButtonYShift == newCollapseButtonYShift)
    return;
  CollapseButtonYShift = newCollapseButtonYShift;
  CollapseButton->setCenterOn(QPoint(RootSplitter->sizes()[0], CollapseButtonYShift));
  emit CollapseButtonYShiftChanged();
}

void ViewRootLayoutPages::showEvent(QShowEvent *event)
{
  CollapseButton->setCenterOn(QPoint(RootSplitter->sizes()[0], 100));
}

void ViewRootLayoutPages::mouseMoveEvent(QMouseEvent *event)
{
}

void ViewRootLayoutPages::UpdateCirclePosition(QPointF pos)
{
  if(CollapseButton->pos().x() - static_cast<int>(pos.x()) < MinumunLenght)
    CollapseButton->setCenterOn(QPoint(
      RootSplitter->sizes()[0], static_cast<int>(pos.y())));
}

bool ViewRootLayoutPages::eventFilter(QObject *watched, QEvent *event)
{
  if(isVisible() && (event->type() == QEvent::Resize && watched == Menu))
  {
    auto e = (QResizeEvent*)event;
    CollapseButton->blockSignals(true);
    if(e->size().width() == 0 && !CollapseButton->IsChecked())
      CollapseButton->setIsChecked(true);
    else if(e->size().width() > 0 && CollapseButton->IsChecked())
      CollapseButton->setIsChecked(false);
    CollapseButton->blockSignals(false);

    if(CollapseButton->IsChecked())
      CollapseButton->setCenterOn(QPoint(CollapseButton->pixmap().width()/2, CollapseButtonYShift));
    else
      CollapseButton->setCenterOn(QPoint(e->size().width(), CollapseButtonYShift));
  }
  return false;
}
}//namespace Views

