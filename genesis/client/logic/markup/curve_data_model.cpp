#include "curve_data_model.h"

namespace GenesisMarkup{

CurveDataModel::CurveDataModel(const QList<double> &newKeys, const QList<double> &newValues)
  : QObject(nullptr)
  , mKeys(newKeys)
  , mValues(newValues)
{

}

CurveDataModel::CurveDataModel()
  : QObject(nullptr)
{

}

const QList<double> &CurveDataModel::keys() const
{
  return mKeys;
}

const QList<double> &CurveDataModel::values() const
{
  return mValues;
}

void CurveDataModel::setKeys(const QList<double> &newKeys)
{
  mKeys = newKeys;
  emit curveDataChanged();
}

void CurveDataModel::setValues(const QList<double> &newValues)
{
  mValues = newValues;
  emit curveDataChanged();
}

double CurveDataModel::maxValueXCoord(double xLeft, double xRight, double* yMaxValue)
{
  auto lerp = [](double x1, double y1, double x2, double y2, double x)->double
  {
    return y1 + (x - x1) * ((y2 - y1) / (x2 - x1));
  };

  auto leftIter = std::lower_bound(mKeys.begin(), mKeys.end(), xLeft);
  if(leftIter == mKeys.end())
    return mKeys.last();
  double leftLerpValue = 0;
  auto leftInd = std::distance(mKeys.begin(), leftIter) - 1;
  if(leftIter == mKeys.begin())
  {
    leftLerpValue = mValues.first();
  }
  else
  {
    leftLerpValue = lerp(mKeys[leftInd], mValues[leftInd], mKeys[leftInd + 1], mValues[leftInd + 1], xLeft);
  }

  auto rightIter = std::lower_bound(mKeys.begin(), mKeys.end(), xRight);
  if(rightIter == mKeys.begin())
    return mKeys.first();
  double rightLerpValue = 0;
  size_t rightInd = 0;
  if(rightIter == mKeys.end())
  {
    rightIter = mKeys.end() - 1;
    rightLerpValue = *rightIter;
    rightInd = std::distance(mKeys.begin(), rightIter);
  }
  else
  {
    rightInd = std::distance(mKeys.begin(), rightIter);
    rightLerpValue = lerp(mKeys[rightInd - 1], mValues[rightInd - 1], mKeys[rightInd], mValues[rightInd], xRight);
  }
  double maxValue = leftLerpValue;
  double maxValueXCoord = xLeft;
  for(size_t i = leftInd; i < rightInd - 1; i++)
  {
    if(mValues[i] > maxValue)
    {
      maxValue = mValues[i];
      maxValueXCoord = mKeys[i];
    }
  }
  if(rightLerpValue > maxValue)
  {
    maxValue = rightLerpValue;
    maxValueXCoord = xRight;
  }
  if(yMaxValue)
    *yMaxValue = maxValue;
  return maxValueXCoord;
}
}//GenesisMarkup
