#ifndef MODULE_PLATE_H
#define MODULE_PLATE_H

#include <QWidget>

namespace Ui {
class ModulePlate;
}

class QParallelAnimationGroup;

class ModulePlate : public QWidget
{
  Q_OBJECT

public:
  explicit ModulePlate(QWidget *parent = nullptr);
  ~ModulePlate();

  QColor HeadColor() const;
  void setHeadColor(const QColor &newHeadColor);
  QPixmap Pixmap() const;
  void setPixmap(const QPixmap &newPixmap);
  QString Title() const;
  void setTitle(const QString &newTitle);
  bool Available() const;
  void setAvailable(bool newAvailable);
  QString Details() const;
  void setDetails(const QString &newDetails);
  int AnimationDuration() const;
  void setAnimationDuration(int newAnimationDuration);
  QPixmap ActivePixmap() const;
  void setActivePixmap(const QPixmap &newActivePixmap);
  QPixmap InactivePixmap() const;
  void setInactivePixmap(const QPixmap &newInactivePixmap);
  QColor CurrentHeadColor() const;
  void setCurrentHeadColor(const QColor &newCurrentHeadColor);

signals:
  void HeadColorChanged();
  void PixmapChanged();
  void TitleChanged();
  void AvailableChanged();
  void DetailsChanged();
  void AnimationDurationChanged();
  void CurrentHeadColorChanged();
  void ActivePixmapChanged();
  void InactivePixmapChanged();
  void clicked();

private:
  Ui::ModulePlate *ui;
  QColor mHeadColor;
  QColor mCurrentHeadColor;
  QPixmap mPixmap;
  QPixmap mInactivePixmap;
  QPixmap mActivePixmap;
  QString mTitle;
  bool mAvailable;
  QString mDetails;
  int mAnimationDurationMs;
  const QColor mInactiveHeadColor;

  QParallelAnimationGroup* mHoverInactiveAnimation;
  QParallelAnimationGroup* mHoverActiveAnimation;

private:
  void setupUi();
  Q_PROPERTY(QColor HeadColor READ HeadColor WRITE setHeadColor NOTIFY HeadColorChanged FINAL)
  Q_PROPERTY(QPixmap Pixmap READ Pixmap WRITE setPixmap NOTIFY PixmapChanged FINAL)
  Q_PROPERTY(QString Title READ Title WRITE setTitle NOTIFY TitleChanged FINAL)
  Q_PROPERTY(bool Available READ Available WRITE setAvailable NOTIFY AvailableChanged FINAL)
  Q_PROPERTY(QString Details READ Details WRITE setDetails NOTIFY DetailsChanged FINAL)
  Q_PROPERTY(int AnimationDuration READ AnimationDuration WRITE setAnimationDuration NOTIFY AnimationDurationChanged FINAL)
  Q_PROPERTY(QColor CurrentHeadColor READ CurrentHeadColor WRITE setCurrentHeadColor NOTIFY CurrentHeadColorChanged FINAL)
  Q_PROPERTY(QPixmap ActivePixmap READ ActivePixmap WRITE setActivePixmap NOTIFY ActivePixmapChanged FINAL)
  Q_PROPERTY(QPixmap InactivePixmap READ InactivePixmap WRITE setInactivePixmap NOTIFY InactivePixmapChanged FINAL)

protected:
  void paintEvent(QPaintEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
};

#endif // MODULE_PLATE_H
