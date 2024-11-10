#include "pls_statistic_widget.h"
#include "qpainter.h"
#include "ui_pls_statistic_widget.h"

#include <genesis_style/style.h>
#include <QGraphicsDropShadowEffect>
#include <ui/itemviews/datatable_item_delegate.h>


namespace Widgets
{
  PlsStatisticWidget::PlsStatisticWidget(QWidget* parent)
    :QWidget(parent)
  {
    WidgetUi = new Ui::PlsStatisticWidget();
    WidgetUi->setupUi(this);
    this->setStyleSheet(Style::statisticsPlateStyle());

    WidgetUi->caption->setStyleSheet(Style::Genesis::GetH2());
    QString style = Style::statisticsTableStyle();
    WidgetUi->statisticTableView->setStyleSheet(style);
    WidgetUi->statisticTableView->horizontalHeader()->setSectionsMovable(false);
    WidgetUi->statisticTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    //delegate swap
    auto oldDelegate = WidgetUi->statisticTableView->itemDelegate();
    WidgetUi->statisticTableView->setItemDelegate(new DataTableItemDelegate(WidgetUi->statisticTableView));
    if(oldDelegate)
      oldDelegate->deleteLater();

    StatisticModel = std::make_shared<StatisticTableModel>();
    WidgetUi->statisticTableView->setModel(StatisticModel.get());

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
    effect->setBlurRadius(50);
    effect->setXOffset(0);
    effect->setYOffset(0);
    effect->setColor(Qt::gray);

    this->setGraphicsEffect(effect);
  }

  PlsStatisticWidget::~PlsStatisticWidget()
  {
    delete WidgetUi;
  }

  void PlsStatisticWidget::SetStatistic(const Structures::PlsStatistic *statisticData)
  {
    StatisticModel->SetStatistic(statisticData);
//    WidgetUi->statisticTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    adjustSize();
  }
}


QString Style::statisticsTableStyle()
{
  QString style =
"QTableView \n"
"{\n"
"  outline:                      none;\n"
"  color:                        none;\n"
"  background-color:             none;\n"
"  border:                       none;\n"
"}\n"
"QTableView::item \n"
"{\n"
"  outline:                      none;\n"
"  color:                        none;\n"
"  background-color:             none;\n"
"  border:                       none;\n"
"  padding:                      @tableViewItemPadding;\n"
"}\n"
"\n"
"QHeaderView \n"
"{\n"
"  qproperty-defaultAlignment:   AlignLeft AlignTop;\n"

"  font:                         @defaultFontBold;\n"

"  color:                        @textColor;\n"
"  background-color:             white;\n"
"}\n"
"\n"
"QHeaderView::section:horizontal\n"
"{\n"
"  padding:                      @tableViewItemPadding;\n"

"  font:                         @defaultFontBold;\n"

"  color:                        @textColor;\n"
"  background-color:             white;\n"

"  border:                       1px solid @frameColorViewGridOpaque;\n"

"  border-left-color:            transparent;\n"
"  border-right-color:           transparent;\n"
"  border-top-color:             transparent;\n"
"}\n"
"QHeaderView::section:vertical\n"
"{\n"
"  padding:                      @tableViewItemPadding;\n"

"  font:                         @defaultFontBold;\n"

"  color:                        @textColor;\n"
"  background-color:             white;\n"

"  border:                       none;\n"
"}\n"
"\n"
"TableViewHeaderView::down-arrow {\n"
"  width:  1px;\n"
"  height: 1px;\n"
"  subcontrol-position: right center;\n"
"  subcontrol-origin: content;\n"
"  image:  url(:/resource/styles/genesis/null/null.png);\n"
"}\n"
"\n"
"TableViewHeaderView::up-arrow {\n"
"  width:  1px;\n"
"  height: 1px;\n"
"  subcontrol-position: right center;\n"
"  subcontrol-origin: content;\n"
"  image: url(:/resource/styles/genesis/null/null.png);\n"
"}\n"
"\n"
"QTableView QWidget\n"
"{\n"
"  background-color:             white;\n"
"  border:                       1px solid @frameColorViewGridOpaque;\n"

"  border-left-color:            transparent;\n"
"  border-right-color:           transparent;\n"
"  border-top-color:             transparent;\n"
"}\n";
  return Style::ApplySASS(style);
}

QString Style::statisticsPlateStyle()
{
  QString style =
      ".QWidget {background-color: white; border: 1px solid gray;}";
  style = Style::ApplySASS(style);
  return style;
}


void Widgets::PlsStatisticWidget::paintEvent(QPaintEvent *event)
{
  QStyleOption opt;
  opt.initFrom(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
