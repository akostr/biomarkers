#pragma once
#ifndef VIEW_PAGE_STATISTIC_H
#define VIEW_PAGE_STATISTIC_H

#include "view.h"

#include <QWidget>
#include <QByteArray>

class QVBoxLayout;
class StatisticWidget;
class QNetworkReply;

namespace Ui
{
  class ViewPageStatistic;
}

namespace Views
{
  class ViewPageStatistic : public View
  {
    Q_OBJECT
  public:
    explicit ViewPageStatistic(QWidget* parent = nullptr);
    ~ViewPageStatistic() = default;

    void ApplyContextUi(const QString&, const QVariant&);

  private:
    void SetupUi();
    void ConnectSignals();

    void downloadClicked();
    void downloadFromStorageClicked();
    void saveFileFromBackend(QNetworkReply* reply, QByteArray fileArray);

    Ui::ViewPageStatistic* mUi = nullptr;
  };
}
#endif