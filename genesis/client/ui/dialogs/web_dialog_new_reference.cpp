#include "web_dialog_new_reference.h"
#include <genesis_style/style.h>

WebDialogNewReference::WebDialogNewReference(QWidget *parent, QString new_name)
    : WebDialog(parent, nullptr, QSizeF(), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
  , parent_widget(parent)
  , reference_name(new_name)
{
    SetupUi();
}


void WebDialogNewReference::SetupUi()
{
    ////Buttons
    {
        if (auto ok = ButtonBox->button(QDialogButtonBox::Ok))
          {
            ok->setDefault(true);
            ok->setText(tr("Apply new reference"));
            ok->setStyleSheet("background-color: red");
          }
    }

    ButtonLayout->setAlignment(Qt::AlignRight);

    //// Body
    Body = new QWidget(Content);
    Body->setMaximumWidth(Style::Scale(520));

    Body->setStyleSheet(Style::Genesis::GetUiStyle());

    Content->layout()->addWidget(Body);

    BodyLayout = new QVBoxLayout(Body);
    BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    BodyLayout->setSpacing(0);

    //// Content
    {
      ////Header
        {
            QLabel* caption = new QLabel(tr("SET NEW REFERENCE"), Body);
            QHBoxLayout * header = new QHBoxLayout();
            caption->setAlignment(Qt::AlignLeft);
            caption->setStyleSheet(Style::Genesis::GetH2());
            header->addWidget(caption);

            header->addSpacing(200);

            QPushButton *exit = new QPushButton();
            exit->setIcon(QIcon(":/resource/icons/icon_action_cross.png"));

            exit->setMinimumSize(20,20);
            exit->setMaximumSize(20,20);

            header->addWidget(exit);

            connect(exit, &QPushButton::clicked, ButtonBox->button(QDialogButtonBox::Cancel), &QPushButton::click);

            BodyLayout->addLayout(header);

            BodyLayout->addSpacing(Style::Scale(15));

        }
      //// Body
      {
            auto form = new QWidget;
            form->setStyleSheet(Style::Genesis::GetUiStyle());

            auto formLayout = new QFormLayout(form);
            formLayout->setContentsMargins(0, 0, 0, 0);
            formLayout->setVerticalSpacing(Style::Scale(10));
            formLayout->setHorizontalSpacing(Style::Scale(80));

            ////
            {
              QLabel* label = new QLabel(tr("Вы действительно хотите назначить хроматограмму \n\"%1\" \nэталоном?\n"
                                            "Разметка хроматограмм \nбудет сброшена.").arg(reference_name), Body);
              BodyLayout->addWidget(label);

            }
        }
    }
}


void WebDialogNewReference::Accept()
{
    WebDialog::Accept();
}

void WebDialogNewReference::Reject()
{
    WebDialog::Reject();
}



