#ifndef IDENTIFICATION_LEGEND_PLATE_H
#define IDENTIFICATION_LEGEND_PLATE_H

#include <QWidget>
#include <QPointer>
class QLabel;
class IdentificationLegendPlate : public QWidget
{
  Q_OBJECT
public:
  explicit IdentificationLegendPlate(const QVariant& color, const QString &text, QWidget *parent = nullptr);
  void setColor(const QVariant& color);
  void setText(const QString& text);

private:
  QPointer<QLabel> mColorLabel;
  QPointer<QLabel> mTextLabel;

};

#endif // IDENTIFICATION_LEGEND_PLATE_H
