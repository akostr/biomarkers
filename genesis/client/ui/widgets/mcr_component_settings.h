#pragma once
#ifndef MCR_COMPONENT_SETTINGS_H
#define MCR_COMPONENT_SETTINGS_H

#include <QWidget>

namespace Ui
{
  class MCRComponentSettings;
}

namespace Widgets
{
  class MCRComponentSettings : public QWidget
  {
    Q_OBJECT

  public:
    MCRComponentSettings(QWidget* parent = nullptr);
    ~MCRComponentSettings();

    void SetNonNegativeConcentration(Qt::CheckState state);
    void SetNonNegativeSpectral(Qt::CheckState state);
    void SetFixed(Qt::CheckState state);
    void SetSampleList(const QStringList& list);
    void SetSampleListVisible(bool isVisible);

    bool IsNonNegativeConcentration() const;
    bool IsNonNegativeSpectral() const;
    bool IsFixed() const;

    QString GetSelectedSample() const;
    void SetSelectedSample(const QString& sampleName);

    void SetLabelText(const QString& componentName);
    void SetStarVisible(bool visible);

  private:
    Ui::MCRComponentSettings* WidgetUi = nullptr;
  };
}

#endif