#ifndef INTERMARKERS_TRANSFER_DIALOG_H
#define INTERMARKERS_TRANSFER_DIALOG_H

#include "logic/markup/genesis_markup_forward_declare.h"
#include <ui/dialogs/templates/dialog.h>

namespace Ui {
class IntermarkersTransferDialog;
}
namespace GenesisMarkup{
class ChromatogramDataModel;
}
class QComboBox;
class QDoubleSpinBox;
class IntermarkersTransferDialog : public Dialogs::Templates::Dialog
{
  Q_OBJECT

public:
  explicit IntermarkersTransferDialog(GenesisMarkup::ChromatogramDataModel* model, QWidget *parent);
  ~IntermarkersTransferDialog();

  int getParameter();
  QVector <GenesisMarkup::IntermarkerInterval> getIntervals();

signals:
  void valitationRequested();

private:
  Ui::IntermarkersTransferDialog *ui;
  QWidget* mContent;
  QList<QPair<QString, double>> mMarkersSortedList;//<name, retention time>
  bool validationRequested = false;

private:
  void setupUi();
  void handleCombos(int changedRow, bool start);
  QComboBox* getStartCombo(int row);
  QComboBox* getEndCombo(int row);
  QDoubleSpinBox* getSpinbox(int row);
  int findRow(QPushButton* closeBtn);
  int findRow(QComboBox* combo, int column);
  bool isIntervalsValid();
  void validateInput();

private:
  enum IntermarkerParameter
  {
      Index,
      Value,

      Last
  };
};

#endif // INTERMARKERS_TRANSFER_DIALOG_H
