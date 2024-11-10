#pragma once

#include <QProxyStyle>

#include "style.h"

/////////////////////////////////////////////////
//// Fwds
class QAbstractSpinBox;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QPlainTextEdit;
class QComboBox;
class QCheckBox;
class QRadioButton;
class QDialogButtonBox;
class QMessageBox;
class QCommandLinkButton;
class QTableView;
class QLabel;
class QTabBar;
class QToolButton;

#ifndef DISABLE_STYLES
/////////////////////////////////////////////////
//// App Propxy Style
class AppPropxyStyle : public QCommonStyle
{
  Q_OBJECT
  
  typedef QCommonStyle Inherited;
  
public:
  explicit AppPropxyStyle();
  ~AppPropxyStyle();

  //// Impls
  virtual int styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget, QStyleHintReturn* returnData) const override;

  //// Impls
  virtual void drawControl(QStyle::ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const override;
  virtual void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget = 0) const override;
  virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;

  virtual void polish(QPalette& palette) override;
  virtual void polish(QWidget* widget) override;

  virtual QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
  virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt, const QWidget *widget = nullptr) const override;
  
  virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
  virtual void Polish(QAbstractSpinBox* asb);
  virtual void Polish(QLineEdit* le);
  virtual void Polish(QTextEdit* te);
  virtual void Polish(QPlainTextEdit* pte);
  virtual void Polish(QComboBox* cb);
  virtual void Polish(QCommandLinkButton* clb);
  virtual void Polish(QPushButton* pb);
  virtual void Polish(QCheckBox* cb);
  virtual void Polish(QRadioButton* rb);
  virtual void Polish(QDialogButtonBox* dbb);
  virtual void Polish(QMessageBox* mb);
  virtual void Polish(QTableView* tv);
  virtual void Polish(QLabel* lbl);
  virtual void Polish(QTabBar* lbl);
  virtual void Polish(QToolButton* tbtn);

  virtual void PrePolish(QLabel* lbl);

  virtual void PolishInputWidget(QWidget* inputWidget);
};
#else 
typedef QProxyStyle AppPropxyStyle;
#endif // !DISABLE_STYLES
