#include "curve_label_widget.h"

#include <QHBoxLayout>
#include <QPainter>

CurveLabelWidget::CurveLabelWidget(const QString& text, const QColor& color, QWidget* parent)
  : QWidget(parent)
  , m_text(text)
  , m_color(color)
{
  setupUi();
}

CurveLabelWidget::~CurveLabelWidget()
{}

void CurveLabelWidget::setText(const QString& text)
{
  m_textLabel->setText(text);
}

void CurveLabelWidget::setColor(const QColor& color)
{
  m_color = color;
  m_colorLabel->setPixmap(createPixmap(m_color));
}

QString CurveLabelWidget::text() const
{
  return m_text;
}

QColor CurveLabelWidget::color() const
{
  return m_color;
}

void CurveLabelWidget::setupUi()
{
  auto layout = new QHBoxLayout();
  setLayout(layout);
  m_colorLabel = new QLabel();
  layout->addWidget(m_colorLabel);

  m_textLabel = new QLabel(m_text);
  layout->addWidget(m_textLabel);

  m_colorLabel->setScaledContents(false);
  m_colorLabel->setPixmap(createPixmap(m_color));
}

QPixmap CurveLabelWidget::createPixmap(QColor color)
{
  QPixmap linePreview(8, 8);
  linePreview.fill(QColor(0, 0, 0, 0));
  QPainter p(&linePreview);
  p.setPen(QPen(color, 2));
  const auto len = linePreview.size().height();
  p.fillRect(0, 0, len, len, color);
  return linePreview;
}

void CurveLabelWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
//  qDebug() << "EVENT HAPPENED IN CURVELABEL!!";
}
