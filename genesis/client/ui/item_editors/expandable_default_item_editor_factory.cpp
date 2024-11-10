#include "expandable_default_item_editor_factory.h"

// all of code was copied from sources of QDefaultItemEditorFactory and
// a little bit modified in "default" sections of switches

///Default editors
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <limits.h>
#include <float.h>
#include <qapplication.h>
#include <qdebug.h>


class QExpandingLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  QExpandingLineEdit(QWidget *parent);

  void setWidgetOwnsGeometry(bool value)
  {
    widgetOwnsGeometry = value;
  }

protected:
  void changeEvent(QEvent *e) override;

public slots:
  void resizeToContents();

private:
  void updateMinimumWidth();

  int originalWidth;
  bool widgetOwnsGeometry;
};

class QBooleanComboBox : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(bool value READ value WRITE setValue USER true)

public:
  QBooleanComboBox(QWidget *parent);
  void setValue(bool);
  bool value() const;
};


class QUIntSpinBox : public QSpinBox
{
  Q_OBJECT
  Q_PROPERTY(uint value READ uintValue WRITE setUIntValue NOTIFY uintValueChanged USER true)
public:
  explicit QUIntSpinBox(QWidget *parent = nullptr)
    : QSpinBox(parent)
  {
    connect(this, SIGNAL(valueChanged(int)), SIGNAL(uintValueChanged()));
  }

  uint uintValue()
  {
    return value();
  }

  void setUIntValue(uint value_)
  {
    return setValue(value_);
  }

signals:
  void uintValueChanged();
};
///Defauls editors end


ExpandableDefaultItemEditorFactory::ExpandableDefaultItemEditorFactory() {}


QWidget *ExpandableDefaultItemEditorFactory::createEditor(int userType, QWidget *parent) const
{
  switch (userType) {
  case QMetaType::Bool: {
    QBooleanComboBox *cb = new QBooleanComboBox(parent);
    cb->setFrame(false);
    cb->setSizePolicy(QSizePolicy::Ignored, cb->sizePolicy().verticalPolicy());
    return cb; }
  case QMetaType::UInt: {
    QSpinBox *sb = new QUIntSpinBox(parent);
    sb->setFrame(false);
    sb->setMinimum(0);
    sb->setMaximum(INT_MAX);
    sb->setSizePolicy(QSizePolicy::Ignored, sb->sizePolicy().verticalPolicy());
    return sb; }
  case QMetaType::Int: {
    QSpinBox *sb = new QSpinBox(parent);
    sb->setFrame(false);
    sb->setMinimum(INT_MIN);
    sb->setMaximum(INT_MAX);
    sb->setSizePolicy(QSizePolicy::Ignored, sb->sizePolicy().verticalPolicy());
    return sb; }
  case QMetaType::QDate: {
    QDateTimeEdit *ed = new QDateEdit(parent);
    ed->setFrame(false);
    return ed; }
  case QMetaType::QTime: {
    QDateTimeEdit *ed = new QTimeEdit(parent);
    ed->setFrame(false);
    return ed; }
  case QMetaType::QDateTime: {
    QDateTimeEdit *ed = new QDateTimeEdit(parent);
    ed->setFrame(false);
    return ed; }
  case QMetaType::QPixmap:
    return new QLabel(parent);
  case QMetaType::Double: {
    QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
    sb->setFrame(false);
    sb->setMinimum(-DBL_MAX);
    sb->setMaximum(DBL_MAX);
    sb->setDecimals(6);
    sb->setSizePolicy(QSizePolicy::Ignored, sb->sizePolicy().verticalPolicy());
    return sb; }
  case QMetaType::QString:{
    QExpandingLineEdit *le = new QExpandingLineEdit(parent);
    le->setFrame(le->style()->styleHint(QStyle::SH_ItemView_DrawDelegateFrame, nullptr, le));
    if (!le->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, nullptr, le))
      le->setWidgetOwnsGeometry(true);
    return le; }
  default:
    return QItemEditorFactory::createEditor(userType, parent);
  }
  return nullptr;
}

QByteArray ExpandableDefaultItemEditorFactory::valuePropertyName(int userType) const
{
  switch (userType) {
  case QMetaType::Bool:
    return "currentIndex";
  case QMetaType::UInt:
  case QMetaType::Int:
  case QMetaType::Double:
    return "value";
  case QMetaType::QDate:
    return "date";
  case QMetaType::QTime:
    return "time";
  case QMetaType::QDateTime:
    return "dateTime";
  case QMetaType::QString:
    return "text";
  default:
    return QItemEditorFactory::valuePropertyName(userType);
  }
}



QExpandingLineEdit::QExpandingLineEdit(QWidget *parent)
  : QLineEdit(parent), originalWidth(-1), widgetOwnsGeometry(false)
{
  connect(this, SIGNAL(textChanged(QString)), this, SLOT(resizeToContents()));
  updateMinimumWidth();
}

void QExpandingLineEdit::changeEvent(QEvent *e)
{
  switch (e->type())
  {
  case QEvent::FontChange:
  case QEvent::StyleChange:
  case QEvent::ContentsRectChange:
    updateMinimumWidth();
    break;
  default:
    break;
  }

  QLineEdit::changeEvent(e);
}

void QExpandingLineEdit::updateMinimumWidth()
{
  const QMargins tm = textMargins();
  const QMargins cm = contentsMargins();
  const int width = tm.left() + tm.right() + cm.left() + cm.right() + 4 /*horizontalMargin in qlineedit.cpp*/;

  QStyleOptionFrame opt;
  initStyleOption(&opt);

  int minWidth = style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(width, 0), this).width();
  setMinimumWidth(minWidth);
}

void QExpandingLineEdit::resizeToContents()
{
  int oldWidth = width();
  if (originalWidth == -1)
    originalWidth = oldWidth;
  if (QWidget *parent = parentWidget()) {
    QPoint position = pos();
    int hintWidth = minimumWidth() + fontMetrics().horizontalAdvance(displayText());
    int parentWidth = parent->width();
    int maxWidth = isRightToLeft() ? position.x() + oldWidth : parentWidth - position.x();
    int newWidth = qBound(qMin(originalWidth, maxWidth), hintWidth, maxWidth);
    if (widgetOwnsGeometry)
      setMaximumWidth(newWidth);
    if (isRightToLeft())
      move(position.x() - newWidth + oldWidth, position.y());
    resize(newWidth, height());
  }
}

QBooleanComboBox::QBooleanComboBox(QWidget *parent)
  : QComboBox(parent)
{
  addItem(QComboBox::tr("False"));
  addItem(QComboBox::tr("True"));
}

void QBooleanComboBox::setValue(bool value)
{
  setCurrentIndex(value ? 1 : 0);
}

bool QBooleanComboBox::value() const
{
  return (currentIndex() == 1);
}

#include "expandable_default_item_editor_factory.moc"
