#ifndef WEBDIALOGPCALEGENDSETTINGS_H
#define WEBDIALOGPCALEGENDSETTINGS_H

#include <QMap>
#include <ui/dialogs/templates/dialog.h>
//#include "web_dialog.h"
#include <graphicsplot/graphicsplot_extended.h>
#include "logic/structures/pca_data_structures.h"
#include <ui/plots/counts_plot.h>

class QTabWidget;
class QComboBox;
class CountsPlot;
namespace Dialogs
{
class WebDialogPCALegendSettings : public Dialogs::Templates::Dialog
{
  Q_OBJECT
public:
  WebDialogPCALegendSettings(QWidget* parent, CountsPlot *plot);

public slots:
  virtual void Accept() override;

private:
  void SetupUi();
  void SetupShapes();
  void SetupColors();

  //helpers
  void AddRow(QGridLayout* l, QString labelText, QWidget* w);
  void AddRow(QGridLayout *l, QLabel *label, QWidget *w);

  void SetStackedLayoutIndex(QStackedLayout* l, ParameterType param);

  QPointer<QTabWidget> TabWidget;
  QPointer<QScrollArea> ShapeTab;
  QPointer<QScrollArea> ColorTab;
  QPointer<QComboBox> ShapeGroupCombo;
  QPointer<QComboBox> ColorGroupCombo;
  QPointer<QStackedLayout> ShapeStackedLayout;
  QPointer<QStackedLayout> ColorStackedLayout;
  QPointer<QCheckBox> DontChangeUserStyleCheckBox;

  QSize ComboIconSize;
  QList<QPair<QIcon, GPShape::ShapeType>> ShapesList;
  QList<QPair<QIcon, GPShapeItem::DefaultColor>> ColorsList;

  CountsPlot* Plot;
  ShapeStyleType ShapeStyle;
  ColorStyleType ColorStyle;
};
}//namespace Dialogs
#endif // WEBDIALOGPCALEGENDSETTINGS_H
