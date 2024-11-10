#include "web_dialog_create_markup.h"

#include <api/api_rest.h>
#include <logic/context_root.h>
#include <logic/known_context_tag_names.h>

#include <genesis_style/style.h>
#include <logic/context_root.h>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QDebug>

using namespace Core;

/////////////////////////////////////////////////////
//// Web Dialog / create markup
WebDialogCreateMarkup::WebDialogCreateMarkup(QWidget* parent)
  : WebDialogCreate(parent, QSizeF(0.4, 0))
{
  SetupModels();
  SetupUi();
}

WebDialogCreateMarkup::~WebDialogCreateMarkup()
{
}

void WebDialogCreateMarkup::SetupModels()
{
  Markups = new TreeModelDynamicProjectMarkupsList(this);
  SetModel(Markups, TreeModelDynamicProjectMarkupsList::ColumnTitle);
}

void WebDialogCreateMarkup::SetupUi()
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
      QLabel* caption = new QLabel(tr("Create markup"), Body);
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
        QLabel* caption = new QLabel(tr("Markup name %1").arg(Style::GetInputAlert()), Body);
        BodyLayout->addWidget(caption);

        BodyLayout->addSpacing(Style::Scale(4));

        MarkupName = new QLineEdit(Body);
        MarkupName->setPlaceholderText(tr("enter markup name"));
        MarkupName->setFocus();
        BodyLayout->addWidget(MarkupName);
      }
    }
  }

  //// Buttons
  if (auto apply = ButtonBox->button(QDialogButtonBox::Apply))
  {
    apply->setText(tr("Create markup"));
    apply->setDefault(true);

    connect(apply, &QPushButton::clicked, ButtonBox, &QDialogButtonBox::accepted);
  }

  //// Connections
  connect(MarkupName, &QLineEdit::textChanged, this, &WebDialogCreateMarkup::UpdateButtons);
  UpdateButtons();
}

QString WebDialogCreateMarkup::GetCurrentName() const
{
  return MarkupName->text();
}

void WebDialogCreateMarkup::Accept()
{
  API::REST::AddMarkup(GenesisContextRoot::Get()->ProjectId(),
                         MarkupName->text(),
  [this](QNetworkReply*, QJsonDocument) {
    TreeModel::ResetInstances("TreeModelDynamicProjectMarkupsList");
    WebDialog::Accept();
  },
  [this](QNetworkReply*, QNetworkReply::NetworkError err) {
    qDebug() << err;
    WebDialog::Accept();
  });
}
