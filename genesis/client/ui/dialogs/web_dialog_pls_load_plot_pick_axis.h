#pragma once
#ifndef PLS_LOAD_PLOT_PICK_AXIS_H
#define PLS_LOAD_PLOT_PICK_AXIS_H

#include "web_dialog.h"

namespace Ui
{
  class WebDialogPlsLoadPlotPickAxis;
}

namespace Dialogs
{
  enum class SelectedTypeView
  {
    DIMENSION2D,
    SPECTRAL
  };

  enum class SpectralGraphicType
  {
    DOT_STYLE,
    LINE_STYLE
  };

  class WebDialogPlsLoadPlotPickAxis : public WebDialog
  {
    Q_OBJECT

  public:
    explicit WebDialogPlsLoadPlotPickAxis(QWidget* parent = nullptr);
    ~WebDialogPlsLoadPlotPickAxis() = default;

    void SetPCXAxisNames(const QStringList& axis);
    void setPCXAxisIndex(int index);

    void SetPCYAxisNames(const QStringList& axis);
    void setPCYAxisIndex(int index);

    int Dim2XAxisCheckBoxValue();
    int Dim2YAxisCheckBoxValue();
    int VerticalAxisYComboBoxValue();

    void setSpectralTypeGraphic(SpectralGraphicType type);
    SpectralGraphicType getSpectralTypeGraphic();
    SelectedTypeView  getSelectedTypeView();
    void setSelectedTypeView(SelectedTypeView type);

  private:
    Ui::WebDialogPlsLoadPlotPickAxis* WidgetUi = nullptr;

    void SetupUi();
    void ConnectSignals();

    void RadioButtonToggled(bool toggled);
  };
}
#endif