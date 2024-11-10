#include "web_dialog_markers_transfer.h"
#include <genesis_style/style.h>
#include <QRadioButton>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

using namespace Names;

WebDialogMarkersTransfer::WebDialogMarkersTransfer(QWidget *parent_widget)
    : WebDialog(parent_widget, nullptr, QSizeF(), QDialogButtonBox::Ok|QDialogButtonBox::Cancel)
    ,_parent_widget(parent_widget)
{
    SetupUi();
    transfer_parameter = TransferParameter::Height;
}


void WebDialogMarkersTransfer::Accept()
{
    WebDialog::Accept();
}

void WebDialogMarkersTransfer::Reject()
{
    WebDialog::Reject();
}

int WebDialogMarkersTransfer::getParameter()
{
  if(rb_height->isChecked())
    return TransferParameter::Height;
  else if(rb_nearest_height)
    return TransferParameter::NearestHeight;
  else if(rb_time)
    return TransferParameter::Time;
  else
    return TransferParameter::Height;
}

void WebDialogMarkersTransfer::SetupUi()
{
    {
        if (auto ok = ButtonBox->button(QDialogButtonBox::Ok))
          {
            ok->setDefault(true);
            if(Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt() == ModulesContextTags::MReservoir)
            {
              ok->setText(tr("Transfer markers"));
            }
            else
            {
              ok->setText(tr("Transfer peaks"));
            }
          }
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
          QLabel* caption = new QLabel(tr("Transfer markers"), Body);
          if(Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt() == ModulesContextTags::MBiomarkers)
          {
            caption->setText(tr("Transfer peaks"));
          }

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
          BodyLayout->setSpacing(6);

        }
        ////Body
        {

            QLabel *label = new QLabel(tr("Chose markers tranfer's parameter"), Body);
            if(Core::GenesisContextRoot::Get()->GetContextModules()->GetData(Names::ModulesContextTags::kModule).toInt() == ModulesContextTags::MBiomarkers)
            {
              label->setText(tr("Chose peaks tranfer's parameter"));
            }
            BodyLayout->addWidget(label);

            rb_height = new QRadioButton(tr("Height"), Body);
            BodyLayout->addWidget(rb_height);

            rb_height->setChecked(true);

            rb_time = new QRadioButton(tr("Retention time"), Body);
            BodyLayout->addWidget(rb_time);

            rb_time->setChecked(false);

            rb_nearest_height = new QRadioButton(tr("Nearest height time"), Body);
            BodyLayout->addWidget(rb_nearest_height);

            rb_time->setChecked(false);

            BodyLayout->addSpacing(Style::Scale(15));

//            connect(checkbox_height, &QCheckBox::clicked, this, [this,checkbox_height, checkbox_time, checkbox_nearest_height]
//            {
//                if(checkbox_height->isChecked())
//                {
//                    checkbox_time->setCheckState(Qt::Unchecked);
//                    checkbox_nearest_height->setCheckState(Qt::Unchecked);
//                    transfer_parameter = TransferParameter::Height;
//                }
//            });

//            connect(checkbox_time, &QCheckBox::clicked, this, [this,checkbox_height, checkbox_time, checkbox_nearest_height]
//            {
//                if(checkbox_time->isChecked())
//                {
//                    checkbox_height->setCheckState(Qt::Unchecked);
//                    checkbox_nearest_height->setCheckState(Qt::Unchecked);
//                    transfer_parameter = TransferParameter::Time;
//                }
//            });

//            connect(checkbox_nearest_height, &QCheckBox::clicked, this, [this,checkbox_height, checkbox_time, checkbox_nearest_height]
//            {
//              if(checkbox_time->isChecked())
//              {
//                checkbox_time->setCheckState(Qt::Unchecked);
//                checkbox_height->setCheckState(Qt::Unchecked);
//                transfer_parameter = TransferParameter::NearestHeight;
//              }
//            });
        }
    }
}

