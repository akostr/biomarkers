#ifndef CURVEDATAMODEL_H
#define CURVEDATAMODEL_H

#include <QObject>

namespace GenesisMarkup{

class CurveDataModel : public QObject
{
  Q_OBJECT
public:
  CurveDataModel(const QList<double>& newKeys, const QList<double>& newValues);
  CurveDataModel();
  const QList<double>& keys() const;
  const QList<double>& values() const;

  void setKeys(const QList<double>& newKeys);
  void setValues(const QList<double>& newValues);
  double maxValueXCoord(double xLeft, double xRight, double *yMaxValue = nullptr);

signals:
  void curveDataChanged();

private:
  QList<double> mKeys;
  QList<double> mValues;

};
using CurveDataModelPtr = QSharedPointer<CurveDataModel>;
}//GenesisMarkup
#endif // CURVEDATAMODEL_H
