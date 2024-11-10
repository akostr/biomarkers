#ifndef LINE_ENDING_COMBO_BOX_H
#define LINE_ENDING_COMBO_BOX_H

#include <QComboBox>
#include <graphicsplot/graphicsplot.h>

enum LineEndingComboDataRoles
{
  RoleEndingStyle = Qt::UserRole + 1,
  RoleEnding,
};

class LineEndingComboBox : public QComboBox
{
  Q_OBJECT
public:
  LineEndingComboBox(QWidget *parent = nullptr);
  void setCurrentEndingStyle(GPLineEnding::EndingStyle style);
  GPLineEnding::EndingStyle currentEndingStyle();
  GPLineEnding currentLineEnding();
  static void drawEndingStyle(QPainter *painter,
                              const QRect &drawRect,
                              const GPLineEnding &lineEnding,
                              const QColor &background,
                              bool leftToRight = true);

signals:
  void lineEndingChanged(GPLineEnding::EndingStyle style);

  // QWidget interface
protected:
  void paintEvent(QPaintEvent *event) override;
};


#endif // LINE_ENDING_COMBO_BOX_H
