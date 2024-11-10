#include "genesis_window.h"
#include "view_login_create.h"

#include "ui/known_view_names.h"
using namespace ViewPageNames;

#include <genesis_style/style.h>

#include <QLabel>

using namespace Core;
////////////////////////////////////////////////////
//// Login create view class
namespace Views
{
ViewLoginCreate::ViewLoginCreate(QWidget* parent)
  : View(parent)
{
  SetupUi();
}

ViewLoginCreate::~ViewLoginCreate()
{
}

void ViewLoginCreate::SetupUi()
{
  //// Self
  setStyleSheet(Style::Genesis::GetUiStyle());

  //// Root
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(Style::Scale(44), Style::Scale(20), Style::Scale(44), Style::Scale(20));
  setContentsMargins(0, 0, 0, 0);

  //// Grid
  {
    LayoutGrid = new QGridLayout;
    Layout->addLayout(LayoutGrid);

    enum column
    {
      columnLeft,
      columnContent,
      columnRight
    };

    enum row
    {
      rowTop,
      rowContent,
      rowBottom,
    };

    LayoutGrid->setColumnStretch(columnLeft, 1);
    LayoutGrid->setColumnStretch(columnContent, 0);
    LayoutGrid->setColumnStretch(columnRight, 1);

    LayoutGrid->setRowStretch(rowTop, 1);
    LayoutGrid->setRowStretch(rowContent, 0);
    LayoutGrid->setRowStretch(rowBottom, 1);

    //// Inputs
    {
      Login = new QWidget(this);
      LayoutGrid->addWidget(Login, rowContent, columnContent);
      Login->setContentsMargins(0, 0, 0, 0);

      LoginLayout = new QVBoxLayout(Login);
      LoginLayout->setContentsMargins(0, 0, 0, 0);
      LoginLayout->setSpacing(0);

      //// Content
      {
        //// Caption
        {
          QLabel* caption = new QLabel(tr("Create account"), Login);
          caption->setMinimumWidth(Style::Scale(468));
          LoginLayout->addWidget(caption);

          caption->setStyleSheet(Style::Genesis::GetH1());
        }

        //// Space
        LoginLayout->addSpacing(Style::Scale(20));

        //// Subcaption
        {
          QLabel* caption = new QLabel(tr("Already registered? %1").arg(Style::GetHyperlink(tr("Authorize"))), Login);
          connect(caption, &QLabel::linkActivated, [] { GenesisWindow::Get()->ShowPage(ViewLoginPageName); });
          LoginLayout->addWidget(caption);
        }

        //// Space
        LoginLayout->addSpacing(Style::Scale(44));

        //// Fields
        {
          //// Last name
          {
            QLabel* caption = new QLabel(tr("Last name %1").arg(Style::GetInputAlert()), Login);
            LoginLayout->addWidget(caption);

            LoginLayout->addSpacing(Style::Scale(4));

            NameLast = new QLineEdit(Login);
            LoginLayout->addWidget(NameLast);
          }

          //// Space
          LoginLayout->addSpacing(Style::Scale(16));

          //// First name
          {
            QLabel* caption = new QLabel(tr("First name %1").arg(Style::GetInputAlert()), Login);
            LoginLayout->addWidget(caption);

            LoginLayout->addSpacing(Style::Scale(4));

            NameFirst = new QLineEdit(Login);
            LoginLayout->addWidget(NameFirst);
          }

          //// Space
          LoginLayout->addSpacing(Style::Scale(16));

          //// Patronymic
          {
            QLabel* caption = new QLabel(tr("Patronymic"), Login);
            LoginLayout->addWidget(caption);

            LoginLayout->addSpacing(Style::Scale(4));

            NamePatronymic = new QLineEdit(Login);
            LoginLayout->addWidget(NamePatronymic);
          }

          //// Space
          LoginLayout->addSpacing(Style::Scale(16));

          //// Email
          {
            QLabel* caption = new QLabel(tr("Email %1").arg(Style::GetInputAlert()), Login);
            LoginLayout->addWidget(caption);

            LoginLayout->addSpacing(Style::Scale(4));

            Email = new QLineEdit(Login);
            LoginLayout->addWidget(Email);
          }

          //// Space
          LoginLayout->addSpacing(Style::Scale(16));

          //// Password
          {
            QLabel* caption = new QLabel(tr("Password %1").arg(Style::GetInputAlert()), Login);
            LoginLayout->addWidget(caption);

            LoginLayout->addSpacing(Style::Scale(4));

            Password = new QLineEdit(Login);
            Password->setEchoMode(QLineEdit::Password);
            LoginLayout->addWidget(Password);
          }

          //// Space
          LoginLayout->addSpacing(Style::Scale(32));

          //// Accept
          {
            Accept = new QDialogButtonBox(QDialogButtonBox::Ok, Login);
            LoginLayout->addWidget(Accept);

            if (auto ok = Accept->button(QDialogButtonBox::Ok))
            {
              ok->setText(tr("Register"));
            }

            if (auto layout = qobject_cast<QHBoxLayout*>(Accept->layout()))
            {
              if (auto space = layout->itemAt(0))
              {
                layout->removeItem(space);
              }
            }
          }

          //// Handle input
          auto handleInput = [this]()
          {
            Accept->setEnabled(
                 !NameLast->text().isEmpty()
              && !NameFirst->text().isEmpty()
              && !Email->text().isEmpty()
              && !Password->text().isEmpty());
          };
          connect(NameLast,       &QLineEdit::textChanged, handleInput);
          connect(NameFirst,      &QLineEdit::textChanged, handleInput);
          connect(NamePatronymic, &QLineEdit::textChanged, handleInput);
          connect(Email,          &QLineEdit::textChanged, handleInput);
          connect(Password,       &QLineEdit::textChanged, handleInput);
          handleInput();
        }
      }
    }
  }
}
}//namespace Views
