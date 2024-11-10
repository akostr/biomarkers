#pragma once

#include <QFrame>

class QHBoxLayout;
class QLabel;

class WarningsFrame : public QFrame
{
  Q_OBJECT

public:
  explicit WarningsFrame(QWidget *parent = nullptr);
  ~WarningsFrame();

  bool HasErrors() const;

public slots:
  void SetErrors(const QString& text);
  void SetErrors(const QStringList& text);
  void ClearErrors();

signals:
  void WarningsChanged(QString text);

protected:
  QHBoxLayout* Layout;
  QLabel* Icon;
  QLabel* Warnings;
};
