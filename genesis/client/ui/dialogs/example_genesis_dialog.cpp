#include "example_genesis_dialog.h"
#include "ui_example_genesis_dialog.h"

ExampleGenesisDialog::ExampleGenesisDialog(QWidget *parent) :
  Dialogs::Templates::Dialog(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel),
  ui(new Ui::ExampleGenesisDialog),
  mContent(new QWidget(nullptr))
{
  Dialogs::Templates::Dialog::Settings s;
  s.dialogHeader = "My custom header";
  s.buttonsNames = {{QDialogButtonBox::Ok, "Все супер"},
                   {QDialogButtonBox::Cancel, "Все плохо"}};
  s.buttonsProperties = {{QDialogButtonBox::Ok, {{"red", true}}},
                         {QDialogButtonBox::Cancel, {{"blue", true}}}};
  applySettings(s);

  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);
}

ExampleGenesisDialog::~ExampleGenesisDialog()
{
  delete ui;
}
