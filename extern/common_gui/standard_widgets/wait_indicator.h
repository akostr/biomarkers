#pragma once

#include <QLabel>
#include <QIcon>
#include <QVector>

class QTimer;

class WaitIndicator : public QLabel
{
  Q_OBJECT
public:
  explicit WaitIndicator(const QString& text, QWidget* parent = nullptr, int size = 24);
  explicit WaitIndicator(QWidget* parent = nullptr);
  ~WaitIndicator();

  void SetIcons(const QStringList& icons);

public slots:
  void OnFrameTimeout();

private:
  void Initialize();

private:
  int             Size;
  int             IconIndex;
  QVector<QIcon>  Icons;
  QTimer*         AnimationTimer;
};
