#include "line_ending_combo_box.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStylePainter>
#include <QStandardItemModel>
#include <QStyle>

namespace
{
class EndingStyleDelegate : public QStyledItemDelegate
{
public:
  EndingStyleDelegate(QObject* parent = nullptr)
    :QStyledItemDelegate(parent)
  {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
  {
    if(option.state.testFlag(QStyle::State_MouseOver))
      LineEndingComboBox::drawEndingStyle(painter,
                                          option.rect,
                                          index.data(RoleEnding).value<GPLineEnding>(),
                                          Qt::blue);
    else
      LineEndingComboBox::drawEndingStyle(painter,
                                          option.rect,
                                          index.data(RoleEnding).value<GPLineEnding>(),
                                          Qt::white);
  }
};

QMap<int, const char*> translateEndingStyleMap
    {
    {GPLineEnding::EndingStyle::esNone, QT_TRANSLATE_NOOP("ENDING_STYLE", "No ending")},
    {GPLineEnding::EndingStyle::esFlatArrow , QT_TRANSLATE_NOOP("ENDING_STYLE", "Flat arrow")},
    {GPLineEnding::EndingStyle::esSpikeArrow, QT_TRANSLATE_NOOP("ENDING_STYLE", "Spike arrow")},
    {GPLineEnding::EndingStyle::esLineArrow , QT_TRANSLATE_NOOP("ENDING_STYLE", "Line arrow")},
    {GPLineEnding::EndingStyle::esDisc      , QT_TRANSLATE_NOOP("ENDING_STYLE", "Disc")},
    {GPLineEnding::EndingStyle::esSquare    , QT_TRANSLATE_NOOP("ENDING_STYLE", "Square")},
    {GPLineEnding::EndingStyle::esDiamond   , QT_TRANSLATE_NOOP("ENDING_STYLE", "Diamond")},
    {GPLineEnding::EndingStyle::esBar       , QT_TRANSLATE_NOOP("ENDING_STYLE", "Bar")},
    {GPLineEnding::EndingStyle::esHalfBar   , QT_TRANSLATE_NOOP("ENDING_STYLE", "Half bar")},
    {GPLineEnding::EndingStyle::esSkewedBar , QT_TRANSLATE_NOOP("ENDING_STYLE", "Skewed bar")}
    };
};

LineEndingComboBox::LineEndingComboBox(QWidget *parent)
  : QComboBox(parent)
{
  connect(this, &QComboBox::currentIndexChanged, this, [this]()
          {
            emit lineEndingChanged(currentData(RoleEndingStyle).value<GPLineEnding::EndingStyle>());
          });
  auto delegate = new EndingStyleDelegate(this);
  setItemDelegate(delegate);
  auto m = qobject_cast<QStandardItemModel*>(QComboBox::model());
  for(int i = GPLineEnding::EndingStyle::esNone; i <= GPLineEnding::EndingStyle::esSkewedBar; i++)
  {
    GPLineEnding::EndingStyle style = (GPLineEnding::EndingStyle)i;
    auto title = qApp->translate("ENDING_STYLE",translateEndingStyleMap[i]);
    GPLineEnding lineEnding(style);
    auto item = new QStandardItem(title);
    item->setData(QVariant::fromValue(style), RoleEndingStyle);
    item->setData(QVariant::fromValue(lineEnding), RoleEnding);
    m->appendRow(item);
  }
  setCurrentIndex(0);
}

void LineEndingComboBox::setCurrentEndingStyle(GPLineEnding::EndingStyle style)
{
  auto ind = findData(QVariant::fromValue(style), RoleEndingStyle);
  setCurrentIndex(ind);
}

GPLineEnding::EndingStyle LineEndingComboBox::currentEndingStyle()
{
  return currentData(RoleEndingStyle).value<GPLineEnding::EndingStyle>();
}

GPLineEnding LineEndingComboBox::currentLineEnding()
{
  return currentData(RoleEnding).value<GPLineEnding>();
}

void LineEndingComboBox::paintEvent(QPaintEvent *event)
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
  drawEndingStyle(&painter, rect, currentLineEnding(), Qt::white);

  // draw the icon and text
  // painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void LineEndingComboBox::drawEndingStyle(QPainter *painter, const QRect &drawRect, const GPLineEnding &lineEnding, const QColor &background, bool leftToRight)
{
  painter->save();
  //background
  painter->setPen(Qt::NoPen);
  painter->setBrush(background);
  painter->drawRect(drawRect);

  //foreground
  painter->setPen(Qt::black);
  painter->setBrush(Qt::black);
  auto center = drawRect.center();

  QPoint left(drawRect.left(), center.y());
  if(!leftToRight
      && lineEnding.style() != GPLineEnding::esNone)
  {
    left.setX(left.x() + lineEnding.length()/2);
  }

  QPoint right(drawRect.right(), center.y());
  if(leftToRight
      &&lineEnding.style() != GPLineEnding::esNone)
    right.setX(right.x() - lineEnding.length()/2);

  painter->drawLine(left, right);
  if(leftToRight)
    lineEnding.draw(painter, GPVector2D(right.x(), center.y()), {1,0});
  else
    lineEnding.draw(painter, GPVector2D(left.x(), center.y()), {-1,0});

  painter->restore();
}
