#include "web_dialog_create_base.h"

#include <genesis_style/style.h>

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

WebDialogCreateBase::WebDialogCreateBase(QWidget* parent, 
                                         const QString& subjectAccusative, 
                                         const QString& subjectGenetive, 
                                         HandleFunc func)
  : WebDialogCreate(parent, QSizeF(0.4, 0))
  , Handle(func)
{
  //// Body
  Body = new QWidget(Content);
  Content->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  BodyLayout->setSpacing(0);

  //// Content
  {
    //// Header
    {
      QLabel* caption = new QLabel(tr("Create %1").arg(subjectAccusative), Body);
      BodyLayout->addWidget(caption);

      caption->setAlignment(Qt::AlignCenter);
      caption->setStyleSheet(Style::Genesis::GetH2());
    }

    //// Space
    BodyLayout->addSpacing(Style::Scale(26));

    //// Input
    {
      //// Name
      {
        QLabel* caption = new QLabel(tr("%1 name %2").arg(subjectGenetive).arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        Name = new QLineEdit(Body);
        Name->setPlaceholderText(tr("enter %1 name").arg(subjectGenetive));
        Name->setFocus();
        BodyLayout->addWidget(Name);
      }
    }
  }

  //// Buttons
  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setText(tr("Create %1").arg(subjectAccusative));
    apply->setDefault(true);

    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  //// Connections
  connect(Name, &QLineEdit::textChanged, this, &WebDialogCreateBase::UpdateButtons);
  UpdateButtons();
}

WebDialogCreateBase::~WebDialogCreateBase()
{
}

QString WebDialogCreateBase::GetCurrentName() const
{
  return Name->text();
}

void WebDialogCreateBase::Accept()
{
  if (Handle)
    Handle(GetCurrentName());
  WebDialogCreate::Accept();
}
