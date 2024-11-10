#ifndef IDENTIFICATION_TREE_MODEL_H
#define IDENTIFICATION_TREE_MODEL_H

#include <logic/tree_model.h>

class IdentificationTreeModel final: public TreeModel
{
  Q_OBJECT
public:
  struct MatchData;
  struct KeyData;

  explicit IdentificationTreeModel(QObject *parent = nullptr);

public:
  // TreeModel interface
  void Load(const QJsonObject &json) override;

  // QAbstractItemModel interface
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role) override;

  void setAlredyUsedCompoundsIds(const QSet<int> &usedIds);

public slots:
  void setCurrentKey(double key);

signals:
  void additionalSpectreAdded(const QVector<double>& mass, const QVector<double>& intencity, int row);
  void additionalSpectreRemoved(int row);
  void spectersCleared();
  void currentSpectreChanged(const QVector<double>& massSpectre, const QVector<double>& intensitySpectre);

private:
  QHash<double, KeyData> mKeysData;
  double currentKey;
  QSet<int> mCheckedRows;
  QSet<int> mUsedCompoundsIds;

public:
  enum Column
  {
    CHECKED,
    METRICS,
    // FAVOURITES,
    // GROUP,
    SHORT_TITLE,
    FULL_TITLE,
    CLASSIFIER_TITLE,
    COMPOUND_CLASS_TITLE,
    TIC_SIM,
    MZ,
    // FORMULA,
    CHEMICAL_FORMULA,
    // SPECIFIC,
    DIMENSION,
    // ANALYSIS_METHODOLOGY,
    // LITERATURE_SOURCE,
    // BIBLIOGRAPHICAL_REFERENCE,
    // MASS_SPECTRUM,
    // INTENSITY_SPECTRUM,
    // PDF_ATTACHED,
    // XLSX_ATTACHED,
    // SPECIFIC_ID,

    // PDF_TITLE,
    // XLSX_TITLE,
    // PDF_SIZE,
    // PDF_LOAD_DATE,
    // XLSX_SIZE,
    // XLSX_LOAD_DATE,
    // ALL_GROUPS_TITLES,
    // LIBRARY_GROUP_ID,
    // LIBRARY_ELEMENT_ID,
    // CLASSIFIER_ID,
    // COMPOUND_CLASS_TYPE_ID,
    LAST_COLUMN,
  };
  struct MatchData
  {
    QString title;
    double metrics;
  };
  struct KeyData
  {
    QVector<double> intensity;
    QVector<double> mass;
    QHash<int, MatchData> matchData;
  };


  // QAbstractItemModel interface
public:
  Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // IDENTIFICATION_TREE_MODEL_H
