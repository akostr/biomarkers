#include "web_dialog_step_back.h"
#include <genesis_style/style.h>


WebDialogStepBack::WebDialogStepBack
    (QWidget *parent, int step)
    : WebDialog(parent, nullptr, QSizeF(), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
    , parent_widget(parent)
    , step_back(step)
{
    SetupUi();
}

void WebDialogStepBack::SetupUi()
{
    SetButtonName(QDialogButtonBox::Ok, tr("Reset and step back"));

    ////Buttons
    {
        if (auto ok = ButtonBox->button(QDialogButtonBox::Ok))
          {
            ok->setDefault(true);
          }
          ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }

    ButtonLayout->setAlignment(Qt::AlignRight);

    //// Body
    Body = new QWidget(Content);

    Body->setStyleSheet(Style::Genesis::GetUiStyle());

    Content->layout()->addWidget(Body);

    BodyLayout = new QVBoxLayout(Body);
    BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    BodyLayout->setSpacing(0);

    //// Content
    {
      ////Header
        {
            QLabel* caption = new QLabel(tr("RETURN TO STEP %1").arg(step_back), Body);
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
            QLabel* label = new QLabel(tr("Вы действительно хотите вернуться на %1 шаг?\n"
                                                   "При переходе будут сброшены все совершенные\nпосле этого шага действия с хроматограммами.").arg(step_back), Body);
            BodyLayout->addWidget(label);


            QLabel* label_1 = new QLabel(tr("Активируйте чекбокс, чтобы подтвердить возврат"), Body);
            BodyLayout->addWidget(label_1);

            label_1->setStyleSheet("font-weight: bold;");

            BodyLayout->addWidget(label_1);

            QCheckBox * checkbox = new QCheckBox(tr("Я подтверждаю, что хочу вернуться на шаг %1 \nи сбросить изменения разметки").arg(step_back), Body);
            BodyLayout->addWidget(checkbox);

            connect(checkbox, &QCheckBox::stateChanged, [this](int state)
            {
                auto ok = ButtonBox->button(QDialogButtonBox::Ok);
                if (state > 0)
                {
                   ok->setDisabled(false);
                   ok->setStyleSheet("background-color: red");
                }
                else
                {
                   ok->setDisabled(true);
                   ok->setStyleSheet("background-color: white");
                }
            });
        }
    }
}

void WebDialogStepBack::Accept()
{
      WebDialog::Accept();
}

void WebDialogStepBack::Reject()
{
    WebDialog::Reject();
}


