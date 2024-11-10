#pragma once

#include <QWidget>

class PlotTemplateModel;

namespace Ui {
class PlotTemplateWidget;
}

class PlotTemplateWidget : public QWidget
{
  Q_OBJECT

public:
  explicit PlotTemplateWidget(QWidget *parent = nullptr);
  ~PlotTemplateWidget();
  void setModel(PlotTemplateModel *model);
  void setEditMode(bool isEdit);
  void setPlotType(int type);
signals:
  void previousStep();
  void cancel();
  void accept();

protected:
  virtual void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *event) override;


private slots:
  void addPixmap();
  void addPolyLine();
  void addText();
  void addLine();
  void resetAddMode();

  void showTreeWidgetMenu(const QPoint &pos);

private:
  void save(); // unused function for debug
  void load(); // unused function for debug
  void initSizeCombobox();
  void plotSizeSelect();
  void plotSizeChange();
  void updatePlotSize();

private:
  Ui::PlotTemplateWidget *ui;
  PlotTemplateModel *m_model;
};


