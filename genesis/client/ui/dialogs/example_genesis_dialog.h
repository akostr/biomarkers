#ifndef EXAMPLE_GENESIS_DIALOG_H
#define EXAMPLE_GENESIS_DIALOG_H

#include <ui/dialogs/templates/dialog.h>

namespace Ui {
class ExampleGenesisDialog;
}

class ExampleGenesisDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  ExampleGenesisDialog(QWidget *parent);
  ~ExampleGenesisDialog();

private:
  Ui::ExampleGenesisDialog *ui;
  QWidget* mContent;
};

#endif // EXAMPLE_GENESIS_DIALOG_H
