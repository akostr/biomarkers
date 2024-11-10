#include "pen_style_combo_box.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStylePainter>
#include <QStandardItemModel>
#include <QStyle>

class EndingStyleDelegate : public QStyledItemDelegate
{
public:
  EndingStyleDelegate(QObject* parent = nullptr)
    :QStyledItemDelegate(parent)
  {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
  {
    if(option.state.testFlag(QStyle::State_MouseOver))
      PenStyleComboBox::drawPenStyle(painter, option.rect, index.data(RolePen).value<QPen>(), Qt::blue);
    else
      PenStyleComboBox::drawPenStyle(painter, option.rect, index.data(RolePen).value<QPen>(), Qt::white);
  }
};

namespace
{
QMap<int, const char*> translatePenStyleMap
{
{Qt::NoPen, QT_TRANSLATE_NOOP("PEN_STYLE", "No pen")},
{Qt::SolidLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Solid line")},
{Qt::DashLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Dash line")},
{Qt::DotLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Dot line")},
{Qt::DashDotLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Dash dot line")},
{Qt::DashDotDotLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Dash dot dot line")},
{Qt::CustomDashLine, QT_TRANSLATE_NOOP("PEN_STYLE", "Custom dash line")}
};
};

PenStyleComboBox::PenStyleComboBox(QWidget *parent)
  : QComboBox(parent)
{
  connect(this, &QComboBox::currentIndexChanged, this, [this]()
          {
            emit penStyleChanged(currentData(RolePenStyle).value<Qt::PenStyle>());
          });
  auto delegate = new EndingStyleDelegate(this);
  setItemDelegate(delegate);
  auto m = qobject_cast<QStandardItemModel*>(QComboBox::model());
  for(int i = Qt::SolidLine; i <= Qt::DashDotDotLine; i++)
  {
    Qt::PenStyle penStyle = (Qt::PenStyle)i;
    auto penTitle = qApp->translate("PEN_STYLE",translatePenStyleMap[i]);
    auto pen = QPen(Qt::black, 2, penStyle);
    auto item = new QStandardItem(penTitle);
    item->setData(QVariant::fromValue(penStyle), RolePenStyle);
    item->setData(QVariant::fromValue(pen), RolePen);
    m->appendRow(item);
  }
  setCurrentIndex(0);
}

void PenStyleComboBox::setCurrentPenStyle(Qt::PenStyle style)
{
  auto ind = findData(QVariant::fromValue(style), RolePenStyle);
  setCurrentIndex(ind);
}

Qt::PenStyle PenStyleComboBox::currentPenStyle()
{
  return currentData(RolePenStyle).value<Qt::PenStyle>();
}

QPen PenStyleComboBox::currentPen()
{
  return currentData(RolePen).value<QPen>();
}

void PenStyleComboBox::paintEvent(QPaintEvent *event)
{
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));
  auto style = this->style();

  // draw the combobox frame, focusrect and selected etc.
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  painter.drawComplexControl(QStyle::CC_ComboBox, opt);

  //unnessesary
  // if (currentIndex() < 0 && !placeholderText().isEmpty()) {
  //   opt.palette.setBrush(QPalette::ButtonText, opt.palette.placeholderText());
  //   opt.currentText = placeholderText();
  // }

  //draw the line style
  auto rect = style->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
  drawPenStyle(&painter, rect, currentPen(), Qt::white);

  // draw the icon and text
  // painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void PenStyleComboBox::drawPenStyle(QPainter *painter, const QRect &drawRect, const QPen &pen, const QColor &background)
{
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(background);
  painter->drawRect(drawRect);

  painter->setPen(pen);
  auto center = drawRect.center();
  QPoint left(drawRect.left(), center.y());
  QPoint right(drawRect.right(), center.y());
  painter->drawLine(left, right);
  painter->restore();
}
