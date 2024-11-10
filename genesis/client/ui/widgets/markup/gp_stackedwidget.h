#ifndef GPStackedWidget_H
#define GPStackedWidget_H

#include <QStackedWidget>
#include <QPointer>

class ChromatogramPlot;
class QPixmap;
class QLabel;

namespace GenesisMarkup{

class GPStackedWidget : public QStackedWidget
{
  Q_OBJECT
public:
  ///
  /// \brief GPStackedWidget
  /// \param plot ownership transfered to GPStackedWidget
  /// \param parent
  ///
  explicit GPStackedWidget(QWidget *parent = nullptr);

  void setLoadingImage(QPixmap * image);
  void setPlot(ChromatogramPlot* plot);

  void showPlot();
  void hidePlot();

  bool hiddenPlot() const;

protected:
  void paintEvent(QPaintEvent *event) override;

protected:
  bool                        mHiddenPlot;
  QPointer<ChromatogramPlot>  mPlot;
  QPointer<QWidget>           mLoadingWidget;

  QPointer<QLabel>            mLabel;
  QScopedPointer<QPixmap>     mLoadingImage;
};

} //GenesisMarkup
#endif // GPStackedWidget_H
