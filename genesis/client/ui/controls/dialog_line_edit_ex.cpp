#include "dialog_line_edit_ex.h"
#include "ui_dialog_line_edit_ex.h"
#include <genesis_style/style.h>
#include <QRegularExpression>
#include <QStyle>

DialogLineEditEx::DialogLineEditEx(QWidget *parent, const Settings &s) :
  QWidget(parent),
  ui(new Ui::DialogLineEditEx),
  mPMValid("://resource/icons/icon_success_on@2x.png"),
  mPMInvalid("://resource/icons/icon_error@2x.png"),
  mPMUnchecked("://resource/icons/icon_success_off@2x.png")
{
  ui->setupUi(this);
  setupUi();
  applySettings(s);
}

DialogLineEditEx::~DialogLineEditEx()
{
  delete ui;
}

void DialogLineEditEx::applySettings(const Settings &settings)
{
  mSettings = settings;
  auto& s = mSettings;
  ui->validationLabel->setProperty("state", state_none);
  setProperty("state", state_none);

  if(s.textMaxLen > 0 && s.defaultText.length() > s.textMaxLen)
    ui->textEdit->setText(s.defaultText.first(s.textMaxLen));
  else
    ui->textEdit->setText(s.defaultText);

  if(s.textMaxLen == -1)
  {
    ui->countLabel->hide();
  }
  else
  {
    if(!ui->countLabel->isVisible())
      ui->countLabel->show();
    ui->countLabel->setText(QString("%1/%2").arg(ui->textEdit->text().length()).arg(s.textMaxLen));
  }

  if(s.textRequired)
  {
    if(ui->validationLabel->isHidden())
      ui->validationLabel->show();
  }
  else if(!ui->validationLabel->isHidden())
    ui->validationLabel->hide();

  QString symbExpr;
  for(auto& sym : s.forbiddenSymbols)
  {
    if(!symbExpr.isEmpty())
      symbExpr.append('|');
    symbExpr.append('\\');
    symbExpr.append(sym);
  }
  if(!mForbiddenSymbolsExpr)
    mForbiddenSymbolsExpr.reset(new QRegularExpression(symbExpr));
  else
    mForbiddenSymbolsExpr->setPattern(symbExpr);

  if(validateData(ui->textEdit->text(), ui->validationLabel, s.forbiddenStrings, true, s.textTooltipsSet))
    setProperty("state", state_valid);
  else
    setProperty("state", state_invalid);

//  if(s.textRequired)
//    ui->headerLabel->setText(s.textHeader + Style::GetInputAlert());
//  else
//    ui->headerLabel->setText(s.textHeader);
  ui->headerLabel->setText(s.textHeader);

  ui->textEdit->setPlaceholderText(s.textPlaceholder);
  if(s.textMaxLen > 0)
    ui->textEdit->setMaxLength(s.textMaxLen);
  else
    ui->textEdit->setMaxLength(32767);

  ui->textEdit->style()->polish(ui->textEdit);

}

DialogLineEditEx::Settings DialogLineEditEx::settings() const
{
  return mSettings;
}

void DialogLineEditEx::setupUi()
{
  ui->headerLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->countLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->textEdit->setStyleSheet(Style::ApplySASS("QLineEdit {color:@uiInputElementColorText;} QLineEdit[text=\"\"] {color:@uiInputElementColorTextDisabled;}\n"
                                               "QLineEdit[style=\"error\"] {border-color: @errorColor;}\n"
                                               "QLineEdit[style=\"success\"] {border-color: @successColor;}\n"
                                               "QLineEdit:disabled {border-color: lightgray;}"));
  ui->validationLabel->setStyleSheet(Style::ApplySASS("QToolTip { color: #ffffff; background-color: black; border-radius: 4px;}\n"
                                                      "QLabel[style=\"error\"] QToolTip {background-color: @errorColor;}\n"
                                                      "QLabel[style=\"success\"] QToolTip {background-color: @successColor;}\n"
                                                      "QLabel:disabled QToolTip{background-color: lightgray;}"));


  connect(ui->textEdit, &QLineEdit::textChanged, this, [this](const QString& text)
  {
    ui->textEdit->style()->polish(ui->textEdit);
    if(mSettings.textMaxLen > 0)
      ui->countLabel->setText(QString("%1/%2").arg(ui->textEdit->text().length()).arg(mSettings.textMaxLen));
    if(mSettings.textRequired)
      validateData(text, ui->validationLabel, mSettings.forbiddenStrings, false, mSettings.textTooltipsSet);
  });

  connect(ui->textEdit, &QLineEdit::editingFinished, this, [this]()
  {
    ui->textEdit->style()->polish(ui->textEdit);
    if(mSettings.textMaxLen)
      ui->countLabel->setText(QString("%1/%2").arg(ui->textEdit->text().length()).arg(mSettings.textMaxLen));
    if((mSettings.textRequired && validateData(ui->textEdit->text(), ui->validationLabel, mSettings.forbiddenStrings, true, mSettings.textTooltipsSet)) || !mSettings.textRequired)
      setProperty("state", state_valid);
    else
      setProperty("state", state_invalid);
  });
}

bool DialogLineEditEx::validateData(QString text, QLabel *validationLabel, const QSet<QString> &forbiddenSet, bool finished, const EditTooltipsSet &tooltipsSet)
{
  bool ret = false;
  TextState prevValue = (TextState)validationLabel->property("state").toInt();
  auto setState = [this, validationLabel](bool isValid, const QString& tooltip)
  {
    validationLabel->setProperty("state", isValid ? state_valid : state_invalid);
    validationLabel->setToolTip(tooltip);
    validationLabel->setPixmap(isValid ? mPMValid : mPMInvalid);
    ui->textEdit->setProperty("style", isValid ? "success" : "error");
    ui->textEdit->style()->polish(ui->textEdit);
    ui->validationLabel->setProperty("style", isValid ? "success" : "error");
    ui->validationLabel->style()->polish(ui->validationLabel);
  };

  if(!mSettings.textRequired)
  {
    validationLabel->setProperty("state", state_valid);
    validationLabel->setToolTip(QString());
//    validationLabel->setPixmap(QPixmap());
    ui->textEdit->setProperty("style", "success");
    ui->textEdit->style()->polish(ui->textEdit);
    ui->validationLabel->setProperty("style", "");
    ui->validationLabel->style()->polish(ui->validationLabel);
    return true;
  }
  //TODO: on release switch back or optimize:
  if(true)//finished)
  {
    if(text.isEmpty())
    {
      setState(false, tooltipsSet.empty);
    }
    else if(forbiddenSet.contains(text))
    {
      if(prevValue != state_invalid)
        setState(false, tooltipsSet.forbidden);
    }
    else if(mSettings.controlSymbols
               && mForbiddenSymbolsExpr
               && text.contains(*mForbiddenSymbolsExpr))
    {
      if(prevValue != state_invalid)
        setState(false, tooltipsSet.invalidSymbol);
    }
    else
    {
      if(prevValue != state_valid)
        setState(true, tooltipsSet.valid);
      ret = true;
    }
  }
  else
  {
    if(prevValue != state_unchecked)
    {
      validationLabel->setProperty("state", state_unchecked);
      validationLabel->setToolTip(tooltipsSet.notChecked);
      validationLabel->setPixmap(mPMUnchecked);
      ui->textEdit->setProperty("style", "");
      ui->textEdit->style()->polish(ui->textEdit);
      ui->validationLabel->setProperty("style", "");
      ui->validationLabel->style()->polish(ui->validationLabel);
    }
    ret = false;
  }
  if(prevValue != (TextState)validationLabel->property("state").toInt())
    emit validityChanged(ret);
  return ret;
}

bool DialogLineEditEx::checkValidity()
{
  return (mSettings.textRequired && validateData(ui->textEdit->text(), ui->validationLabel, mSettings.forbiddenStrings, true, mSettings.textTooltipsSet)) || !mSettings.textRequired;
}

QString DialogLineEditEx::text()
{
  return ui->textEdit->text();
}

void DialogLineEditEx::SetHeaderLabel(const QString& text)
{
  ui->headerLabel->setText(text);
}
