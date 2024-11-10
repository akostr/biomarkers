#ifndef DIALOG_LINE_EDIT_EX_H
#define DIALOG_LINE_EDIT_EX_H

#include <QWidget>
#include <QSet>

namespace Ui {
class DialogLineEditEx;
}
class QLabel;
class DialogLineEditEx : public QWidget
{
  Q_OBJECT

public:
  struct EditTooltipsSet
  {
    QString empty;
    QString forbidden;
    QString notChecked;
    QString valid;
    QString invalidSymbol;
  };
  struct Settings
  {
    Settings() {};
    QString textHeader               = tr("Text title");
    bool textRequired                = true;
    bool controlSymbols              = false;
    QString textPlaceholder          = tr("Enter text here");
    int textMaxLen                   = 50;
    QString defaultText;
    QSet<QString> forbiddenStrings;
    QList<QChar> forbiddenSymbols = {'\\', '/', '\"', '*', '<', '|', '>'};//just array of symbols
    EditTooltipsSet textTooltipsSet = {tr("Text is empty"),
                                       tr("Text is forbidden"),
                                       tr("Not checked"),
                                       tr("Text is valid"),
                                       tr("Text must not contain the characters '\', '/', '\"', '*', '<', '|', '>'")};
  };
  enum TextState
  {
    state_none = 0,
    state_unchecked,
    state_valid,
    state_invalid
  };
  DialogLineEditEx(QWidget *parent = nullptr, const Settings& s = Settings());
  ~DialogLineEditEx();

  void applySettings(const Settings& settings);
  Settings settings() const;
  bool checkValidity();
  QString text();
  void SetHeaderLabel(const QString& text);

signals:
  void validityChanged(bool isValid);

private:
  Ui::DialogLineEditEx *ui;
  Settings mSettings;
  QPixmap mPMValid;
  QPixmap mPMInvalid;
  QPixmap mPMUnchecked;
  QScopedPointer<QRegularExpression> mForbiddenSymbolsExpr;

private:
  void setupUi();
  bool validateData(QString text, QLabel *validationLabel, const QSet<QString> &forbiddenSet, bool finished, const EditTooltipsSet &tooltipsSet);
};
#endif // DIALOG_LINE_EDIT_EX_H
