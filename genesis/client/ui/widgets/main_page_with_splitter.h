#ifndef MAINPAGEWITHSPLITTER_H
#define MAINPAGEWITHSPLITTER_H

#include <QWidget>

class CircleButton;
class QSplitter;
class MainPageWithSplitter : public QWidget
{
  Q_OBJECT
public:
  explicit MainPageWithSplitter(QWidget *parent = nullptr);

signals:

private:
  CircleButton* mCollapseMenuButton;
  QSplitter* mSplitter;
  int mCollapseButtonVerticalOffset = 100;

private:
  void updateCollapseButtonPosition(int pos);

  // QWidget interface
protected:
  void resizeEvent(QResizeEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // MAINPAGEWITHSPLITTER_H
