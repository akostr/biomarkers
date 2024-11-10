#include "baseline_data_model.h"
#include <QQueue>
#include <api/api_rest.h>
#include <logic/notification.h>

namespace GenesisMarkup{

BaseLineDataModel::BaseLineDataModel(const QMap<int, QPair<QVector<double>, QVector<double> > > &baseLines)
  : QObject(nullptr),
    mBaseLines(baseLines),
    mSmoothFactor(2)
{
}

BaseLineDataModel::BaseLineDataModel()
  : QObject{nullptr}
{

}

bool BaseLineDataModel::addBaseLine(int smoothFactor, const QVector<double> &keys, const QVector<double> &values)
{
  if(mBaseLines.contains(smoothFactor))
    return false;
  mBaseLines[smoothFactor] = {keys, values};
  return true;
}

void BaseLineDataModel::setBaseLine(int smoothFactor, const QVector<double> &keys, const QVector<double> &values)
{
  mBaseLines[smoothFactor] = {keys, values};
  mSmoothFactor = smoothFactor;
  emit onChanged(mBaseLines[mSmoothFactor].first, mBaseLines[mSmoothFactor].second);
}

void BaseLineDataModel::setBaseLine(int smoothFactor, int projectFileId)
{
  if(!mBaseLines.contains(smoothFactor))
  {
    if(projectFileId == -1)
      return;
    static QQueue<int> mQueuedSmoothFactors;
    if(!mQueuedSmoothFactors.contains(smoothFactor))
    {
      mQueuedSmoothFactors.enqueue(smoothFactor);
      API::REST::BaselineData(projectFileId, false, smoothFactor,
                              [this, smoothFactor](QNetworkReply*, QJsonDocument doc)
                              {
                                Q_ASSERT(mQueuedSmoothFactors.head() == smoothFactor);
                                mQueuedSmoothFactors.dequeue();
                                auto jroot = doc.object();
                                bool err = jroot["error"].toBool();
                                if(err)
                                {
                                  Notification::NotifyError(tr("Failed to get baseline %1").arg(smoothFactor), jroot["msg"].toString());
                                  return;
                                }
                                auto jbaselines = jroot["baselines"].toObject();
                                auto jblineArr = jbaselines.begin().value().toArray();
                                QVector<double> keys, values;
                                for(int i = 0; i < jblineArr.size(); i++)
                                {
                                  auto jentry = jblineArr[i].toObject();
                                  keys << jentry["x"].toDouble();
                                  values << jentry["y"].toDouble();
                                }
                                addBaseLine(smoothFactor, keys, values);
                                mSmoothFactor = smoothFactor;
                                emit onChanged(mBaseLines[mSmoothFactor].first, mBaseLines[mSmoothFactor].second);
                              },
                              [smoothFactor](QNetworkReply*, QNetworkReply::NetworkError err)
                              {
                                Notification::NotifyError(tr("Failed to get baseline %1").arg(smoothFactor), err);
                              }, Qt::QueuedConnection);
    }
  }
  else
  {
    mSmoothFactor = smoothFactor;
    emit onChanged(mBaseLines[mSmoothFactor].first, mBaseLines[mSmoothFactor].second);
  }
}

QPair<QVector<double>, QVector<double> > BaseLineDataModel::getBaseLine() const
{
  if(!mBaseLines.contains(mSmoothFactor))
    return {{},{}};
  return mBaseLines[mSmoothFactor];
}

int BaseLineDataModel::getSmoothFactor() const
{
  return mSmoothFactor;
}

void BaseLineDataModel::clear()
{
  mBaseLines.clear();
}

const QMap<int, QPair<QVector<double>, QVector<double> > > &BaseLineDataModel::data() const
{
  return mBaseLines;
}
}//GenesisMarkup
