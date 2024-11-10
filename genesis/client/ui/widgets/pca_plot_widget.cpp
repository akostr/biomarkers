#include "pca_plot_widget.h"

#include <QVBoxLayout>
#include <QLocale>

namespace Widgets
{
  PcaPlotWidget::PcaPlotWidget(QWidget* parent)
    :QWidget(parent)
  {}

  void PcaPlotWidget::setModel(QPointer<AnalysisEntityModel> model)
  {
    mEntityModel = model;
  }

  QString PcaPlotWidget::generateAxisLabel(int number, double explPCsVariance) const
  {
    QLocale locale(QLocale::Russian);
    return tr("PC%1\ (%2\%)")
      .arg(number)
      .arg(locale.toString(explPCsVariance, 'f', 2));
  }
}
