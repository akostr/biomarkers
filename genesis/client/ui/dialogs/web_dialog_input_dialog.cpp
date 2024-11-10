#include "web_dialog_input_dialog.h"

#include <genesis_style/style.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

WebDialogInputDialog::WebDialogInputDialog(QString caption, QString subCaption, DataType type, QWidget *parent)
: WebDialog(parent, nullptr, QSize(0.4, 0), QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
, m_type(type)
, m_caption(caption)
, m_subCaption(subCaption)
{
  setupUi();
}

void WebDialogInputDialog::setupUi()
{
  auto body = new QWidget(Content);
  body->setStyleSheet(Style::Genesis::GetUiStyle());
  Content->layout()->addWidget(body);
  auto bodyLayout = new QVBoxLayout();
  body->setLayout(bodyLayout);

  auto mainCaption = new QLabel(m_caption);
  mainCaption->setStyleSheet(Style::Genesis::GetH1());
  bodyLayout->addWidget(mainCaption);
  bodyLayout->addSpacing(30);

  auto subCaption = new QLabel(m_subCaption);
  subCaption->setStyleSheet(Style::Genesis::GetH3());
  bodyLayout->addWidget(subCaption);

  switch(m_type)
  {
  case String:
  {
    m_stringEdit = new QLineEdit();
    connect(m_stringEdit, &QLineEdit::textEdited, this,
            [this](const QString& text){
                                        ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
                                       });
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    bodyLayout->addWidget(m_stringEdit);
    break;
  }
  default:
    break;
  }
}

WebDialogInputDialog::DataType WebDialogInputDialog::getDataType()
{
  return m_type;
}

QVariant WebDialogInputDialog::getValue()
{
  switch(m_type)
  {
  case String:
  {
    return QVariant(m_stringEdit->text());
  }
  default:
    return QVariant();
  }
}
