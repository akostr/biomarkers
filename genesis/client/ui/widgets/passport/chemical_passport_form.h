#ifndef CHEMICAL_PASSPORT_FORM_H
#define CHEMICAL_PASSPORT_FORM_H

#include <QWidget>
#include <logic/structures/passport_form_structures.h>
#include <QPointer>

namespace Ui {
class ChemicalPassportForm;
}

class QPushButton;
class PassportModelsManager;
class ChemicalPassportForm : public QWidget
{
  Q_OBJECT

public:
  explicit ChemicalPassportForm(QWidget *parent = nullptr);
  ~ChemicalPassportForm();
  Passport::SampleChemical data();
  void setData(const Passport::SampleChemical& data);
  void SetModelsManager(QPointer<PassportModelsManager> manager);

protected slots:
  void onButtonCheckStateChanged();
  void onButtonCheckStateChanged(QPushButton *btn);

public slots:
  void onFluidTypeChanged(Passport::FluidType fluidType);

private:
  Ui::ChemicalPassportForm *ui;
  QPointer<PassportModelsManager> mModelsManager;

  void setupUi();
};

#endif // CHEMICAL_PASSPORT_FORM_H
