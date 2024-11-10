#ifndef WEBDIALOGPCAITEMSETTINGS_H
#define WEBDIALOGPCAITEMSETTINGS_H

//#include <QMap>
#include "ui/dialogs/templates/dialog.h"
#include <graphicsplot/graphicsplot_extended.h>
#include <logic/structures/pca_data_structures.h>
#include <ui/plots/counts_plot.h>

namespace Ui {
class WebDialogPCAItemsSettings;
}

class WebDialogPCAItemsSettings : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  WebDialogPCAItemsSettings(QWidget* parent, QList<GPShapeWithLabelItem*> items, CountsPlot* plot);
  ~WebDialogPCAItemsSettings();

public slots:
  virtual void Accept() override;

private:
  void SetupUi();
  void SetupShapes(const QSize &iconSize);
  void SetupColors(const QSize &iconSize);

  Ui::WebDialogPCAItemsSettings* ui;
  QWidget* mContent;

  QSize ComboIconSize;
  QList<QPair<QIcon, GPShape::ShapeType>> ShapesList;
  QList<QPair<QIcon, GPShapeItem::DefaultColor>> ColorsList;
  QList<GPShapeWithLabelItem*> Items;
  CountsPlot* Plot;
};

#endif // WEBDIALOGPCAITEMSETTINGS_H
