#ifndef PASSPORTCHROMATOGRAMMITEMWIDGET_H
#define PASSPORTCHROMATOGRAMMITEMWIDGET_H

#include <QWidget>
#include <QPointer>

class QLabel;
class PassportChromatogrammItemWidget : public QWidget
{
  Q_OBJECT
public:
  explicit PassportChromatogrammItemWidget(QString chromaName, int percents, QWidget *parent = nullptr);
  void setPercents(int percents);

private:
  QString mChromaName;
  int mPercents;
  QPointer<QLabel> mPercentsLabel;

private:
  void setupUi();
};

#endif // PASSPORTCHROMATOGRAMMITEMWIDGET_H
