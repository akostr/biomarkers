#pragma once
#ifndef PCA_PLOT_WIDGET_H
#define PCA_PLOT_WIDGET_H

#include <QWidget>
#include <QPointer>

class AnalysisEntityModel;
namespace Widgets
{
  class PcaPlotWidget : public QWidget
  {
    Q_OBJECT

  public:
    PcaPlotWidget(QWidget* parent = nullptr);
    virtual ~PcaPlotWidget() = default;
    virtual void setModel(QPointer<AnalysisEntityModel> model);

  protected:
    QPointer<AnalysisEntityModel> mEntityModel;

 protected:
    QString generateAxisLabel(int number, double explPCsVariance) const;
  };

  using PcaPlotWidgetPtr = QSharedPointer<PcaPlotWidget>;
}

#endif
