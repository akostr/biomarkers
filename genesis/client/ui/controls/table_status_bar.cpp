#include "table_status_bar.h"

TableStatusBar::TableStatusBar(QWidget* parent, TreeModel* model, QSortFilterProxyModel* proxy)
  : QLabel(parent)
  , Model(model)
  , Proxy(proxy)
{
  Setup();
}

TableStatusBar::~TableStatusBar()
{
}

void TableStatusBar::Setup()
{
  auto reset = [this]()
  {
    Model->Reset();
  };
  connect(this, &QLabel::linkActivated, reset);

  if(Proxy)
  {
    connect(Proxy, &QAbstractItemModel::rowsInserted,  this, &TableStatusBar::Update);
    connect(Proxy, &QAbstractItemModel::rowsRemoved,   this, &TableStatusBar::Update);
    connect(Proxy, &QAbstractItemModel::modelReset,    this, &TableStatusBar::Update);
    connect(Proxy, &QAbstractItemModel::layoutChanged, this, &TableStatusBar::Update);
  }
  if(Model)
  {
    connect(Model, &QAbstractItemModel::rowsInserted,  this, &TableStatusBar::Update);
    connect(Model, &QAbstractItemModel::rowsRemoved,   this, &TableStatusBar::Update);
    connect(Model, &QAbstractItemModel::modelReset,    this, &TableStatusBar::Update);
    connect(Model, &QAbstractItemModel::layoutChanged, this, &TableStatusBar::Update);
  }
  update();
}

void TableStatusBar::Update()
{
  // qDebug() << sender() << "update model status bar";
  QString text = tr("Items: %1").arg(Proxy ? Proxy->rowCount() : Model->rowCount());
  QString tmpl = QString("<table><tr><td valign=\"middle\">%1</td><td>&nbsp;<a href=\"#\"><img src=\":/resource/icons/icon_action_refresh_small.png\"/></a></td></tr></table>").arg(text);
  setText(tmpl);
}
