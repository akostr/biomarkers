#ifndef MAINMENUSPLITTER_H
#define MAINMENUSPLITTER_H

#include <QSplitter>

class CircleButton;
class MainMenuSplitter : public QSplitter
{
  Q_OBJECT
public:
  MainMenuSplitter(QWidget* parent = nullptr);

  // QWidget interface
protected:
  void mouseMoveEvent(QMouseEvent *event) override;

private:
  CircleButton* mCollapseLeftButton;
};

#endif // MAINMENUSPLITTER_H
