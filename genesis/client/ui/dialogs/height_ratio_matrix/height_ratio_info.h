#ifndef HEIGHT_RATIO_INFO_H
#define HEIGHT_RATIO_INFO_H

#include <ui/dialogs/templates/info.h>

namespace Ui {
class HeightRatioInfo;
}
namespace Dialogs
{
class HeightRatioInfo : public Templates::Info
{
  Q_OBJECT
private:
  struct MatrixData
  {
    MatrixData(const QJsonObject& matrixData);
    bool includeMarkers;
    double maxValue;
    double minValue;
    double windowSize;
    QString markupTitle;
    QString tableTitle;
  };
public:
  explicit HeightRatioInfo(const QJsonObject& matrixData, QWidget *parent = nullptr);
  ~HeightRatioInfo();

private:
  Ui::HeightRatioInfo *ui;
  QWidget* mContent = nullptr;
  MatrixData mData;

private:
  void setupUi();
};
}//namespace Dialogs
#endif // HEIGHT_RATIO_INFO_H
