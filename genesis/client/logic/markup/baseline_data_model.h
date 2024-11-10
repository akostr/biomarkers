#ifndef BASELINEDATAMODEL_H
#define BASELINEDATAMODEL_H

#include <QObject>
#include <QMap>
#include <logic/markup/genesis_markup_enums.h>
namespace GenesisMarkup{

class BaseLineDataModel : public QObject
{
  Q_OBJECT
public:
  BaseLineDataModel(const QMap<int, QPair<QVector<double>, QVector<double>>> &baseLines);
  BaseLineDataModel();
  bool addBaseLine(int smoothFactor, const QVector<double> &keys, const QVector<double> &values);
  void setBaseLine(int smoothFactor, const QVector<double> &keys, const QVector<double> &values);
  void setBaseLine(int smoothFactor, int projectFileId = -1);
  QPair<QVector<double>, QVector<double>> getBaseLine() const;
  int getSmoothFactor() const;
  void clear();
  const QMap<int, QPair<QVector<double>, QVector<double>>> &data() const;

signals:
  void onChanged(const QVector<double> &keys, const QVector<double> &values);

private:
  QMap<int, QPair<QVector<double>, QVector<double>>> mBaseLines;
  int mSmoothFactor;
};
}//GenesisMarkup
#endif // BASELINEDATAMODEL_H
