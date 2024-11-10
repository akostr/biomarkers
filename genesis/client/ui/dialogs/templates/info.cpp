#include "info.h"
#include "ui_info.h"
#include <genesis_style/style.h>
#include <QStyle>

namespace Dialogs
{
  namespace Templates
  {
    //PUT YOUR INTERNAL DIALOG CONTENT INTO ui->externalContentContainer!!!
    //and TODO: rework this awful naming later. for now - i'm done ^_^.
    Info::Info(QWidget* parent, QDialogButtonBox::StandardButtons buttons, QWidget* content)
      : WebDialog(parent, nullptr, QSize(), buttons),
      ui(new Ui::info),
      mInternalContent(new QWidget(Content))
    {
      ui->setupUi(mInternalContent);
      setupUi();
      applySettings(Settings());

      Content->layout()->addWidget(mInternalContent);
      auto layout = new QVBoxLayout(ui->externContentContainer);
      ui->externContentContainer->setLayout(layout);
      layout->setContentsMargins(ui->horizontalLayout->contentsMargins());
      if (content)
        layout->addWidget(content);
    }

    Info::Info(QWidget* parent, const Settings& settings, QDialogButtonBox::StandardButtons buttons, QWidget* content)
      : WebDialog(parent, nullptr, QSize(), buttons),
      ui(new Ui::info),
      mInternalContent(new QWidget(Content))
    {
      ui->setupUi(mInternalContent);
      setupUi();
      applySettings(settings);

      Content->layout()->addWidget(mInternalContent);
      auto layout = new QVBoxLayout(ui->externContentContainer);
      ui->externContentContainer->setLayout(layout);
      layout->setContentsMargins(ui->horizontalLayout->contentsMargins());
      if (content)
        layout->addWidget(content);
    }

    Info::~Info()
    {
      delete ui;
    }

    void Info::applySettings(const Settings& newSettings)
    {
      mSettings = newSettings;
      mSettings.contentHeader = mSettings.contentHeader.toUpper();
      ui->dialogHeaderLabel->setText(mSettings.dialogHeader);
      ui->contentHeaderLabel->setText(mSettings.contentHeader);
      for (auto& b : ButtonBox->buttons())
      {
        auto btnCode = ButtonBox->standardButton(b);

        if (mSettings.buttonsNames.contains(btnCode))
          b->setText(mSettings.buttonsNames[btnCode]);

        if (mSettings.buttonsProperties.contains(btnCode))
        {
          auto map = mSettings.buttonsProperties[btnCode];
          for (auto iter = map.constBegin(); iter != map.constEnd(); iter++)
            b->setProperty(iter.key().toStdString().c_str(), iter.value());
        }
        b->style()->polish(b);
      }
    }

    Info::Settings Info::getCurrentSettings()
    {
      return mSettings;
    }

    QWidget* Info::getContent()
    {
      return ui->externContentContainer;
    }

    void Info::setContentHeader(const QString& content)
    {
      ui->contentHeaderLabel->setText(content);
    }

    void Info::setupUi()
    {
      ui->contentHeaderLabel->setStyleSheet(Style::Genesis::Fonts::ModalHeadSteps());
      ui->contentHeaderLabel->setProperty("color", "secondary");
      ui->dialogHeaderLabel->setStyleSheet(Style::Genesis::Fonts::ModalHead());
      connect(ui->closeButton, &QPushButton::clicked, this, &Info::Reject);
    }

  }
}
