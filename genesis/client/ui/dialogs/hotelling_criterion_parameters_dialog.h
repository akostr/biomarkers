#pragma once

#include "ui/dialogs/templates/info.h"

namespace Ui {
  class HotellingCriterionParametersDialog;
}
class AnalysisEntityModel;
class FlowLayout;
class HotellingCriterionParametersDialog : public Dialogs::Templates::Info
{
  Q_OBJECT

public:
  HotellingCriterionParametersDialog(QPointer<AnalysisEntityModel> model, int step = 0,
                                     QWidget* parent = nullptr);
  ~HotellingCriterionParametersDialog();
  int step() const;
  QHash<QString, QList<QUuid> >& groups();

  // WebDialog interface
public slots:
  void Accept() override;

  int getCountOfPC() const;
  QString getGroupingCriterion() const;
  bool isGroupingByCriteria() const;

protected:
  void setupUi();
  void setupStep(int step, bool initial = false);

private:
  void onPCACriterionIngexChanged(int ind);
  void onGroupCalcChanged(const QHash<QString, QList<QUuid> >& groups);
  QHash<QString, QList<QUuid> > getPcaCriterionGroups(int criteria, QStringList *excludedSamples = nullptr);
  QHash<QString, QList<QUuid> > getPcaGroups(QStringList *excludedSamples = nullptr);
  QHash<QString, QList<QUuid> > getManualGroups(QStringList *excludedSamples = nullptr);

  QString sampleName(const QUuid& uid);
  void fillExcluded(QStringList* excludedSamples, const QHash<QString, QList<QUuid> >& groups);
  int maxPCCount();
  void clearExcluded();
  void setExcluded(const QStringList& excluded);

private:
  QHash<int, QHash<QString, QList<QUuid> >> mPcaCriterionCache;
  QHash<int, QStringList> mPcaCriterionExcludedCache;
  QHash<QString, QList<QUuid> >* mPcaCache = nullptr;
  QStringList* mPcaExcludedCache = nullptr;
  Ui::HotellingCriterionParametersDialog* ui;
  QPointer<AnalysisEntityModel> mModel;
  QPointer<FlowLayout> mExcludedLayout;
  QHash<QString, QList<QUuid> > mGroups;
  int mStep;
  QWidget* mContent;


};
