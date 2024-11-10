#ifndef IDENTIFICATION_WIDGET_H
#define IDENTIFICATION_WIDGET_H

#include <logic/markup/i_markup_tab.h>
#include <logic/markup/genesis_markup_forward_declare.h>
#include <QPointer>
#include <QUuid>

namespace Ui {
class IdentificationWidget;
}
class IdentificationTreeModel;
class FlowLayout;
class IdentificationWidget : public IMarkupTab
{
  Q_OBJECT

public:
  explicit IdentificationWidget(QWidget *parent = nullptr);
  ~IdentificationWidget();

  void setChromaHeight(int height);

private:
  Ui::IdentificationWidget *ui;
  QPointer<IdentificationTreeModel> mIdentificationModel;
  GenesisMarkup::ChromatogrammModelPtr mMasterChroma;
  QHash<int, QWidget*> mLegendItems;
  QWidget* mLegendMainItem = nullptr;
  QPointer<FlowLayout> mLegendLayout;
  QAction* mNameSearchAction = nullptr;
  QAction* mNameSearchClearAction = nullptr;
  QUuid mSelectorUid;

private slots:
  void onCurrentSpectreChanged(const QVector<double>& mass, const QVector<double>& intensity);
  void onAdditionalSpectreAdded(const QVector<double>& mass, const QVector<double>& intensity, int sourceRow);
  void onSpectersCleared();
  void onAdditionalSpectreRemoved(int sourceRow);
  void onCompoundColorChanged(int sourceRow, QVariant color);
  void onSpectrumSwitchCheckStateChanged(bool state);
  void setChromaIntensityVisible(bool visible);
  void setCompoundIntensityVisible(bool visible);
  void onApplyButtonClicked();
  void resortLegendItems();
  void applyCompoundIntoPeak(QModelIndex proxyIndex);
  void clearPeakCompound();
  void applyNameFilter();
  void applyFileTypeFilter();
  void applyClassifierTypeFilter();

private:
  QSet<int> getUsedLibraryGroupIds();
  int mPendingChromaHeight = -1;

  // IMarkupTab interface
public:
  void setMarkupModel(GenesisMarkup::MarkupModelPtr markupModelPtr) override;
  bool isVisibleOnStep(GenesisMarkup::StepInfo step) override;

  // QWidget interface
public:
  // bool eventFilter(QObject *watched, QEvent *event) override;

protected:
  void showEvent(QShowEvent *event) override;

public:
  enum FileTypeFilter
  {
    All,
    System,
    Favorites,
    None
  };
};

#endif // IDENTIFICATION_WIDGET_H
