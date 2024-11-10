#include "dialog_text_edit_ex.h"
#include "ui_dialog_text_edit_ex.h"

#include <QStyle>

#include <genesis_style/style.h>

DialogTextEditEx::DialogTextEditEx(QWidget* parent) :
  QWidget(parent)
  , ui(new Ui::DialogTextEditEx)
{
  ui->setupUi(this);
  setupUi();
}

DialogTextEditEx::~DialogTextEditEx()
{
  delete ui;
}

void DialogTextEditEx::applySettings(const Settings& settings)
{
  mSettings = settings;

  ui->commentLabel->setText(mSettings.textHeader);

  if (!mSettings.defaultText.isEmpty())
  {
    if (mSettings.defaultText.length() > mSettings.textMaxLen)
      ui->commentEdit->setText(mSettings.defaultText.first(mSettings.textMaxLen));
    else
      ui->commentEdit->setText(mSettings.defaultText);
  }

  ui->commentEdit->setPlaceholderText(mSettings.textPlaceholder);
  ui->commentEdit->style()->polish(ui->commentEdit);
}

DialogTextEditEx::Settings DialogTextEditEx::settings() const
{
  return mSettings;
}

QString DialogTextEditEx::text()
{
  return ui->commentEdit->toPlainText();
}

void DialogTextEditEx::setupUi()
{
  ui->commentLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());
  ui->commentCountLabel->setStyleSheet(Style::Genesis::Fonts::RegularLight());

  ui->commentEdit->setStyleSheet(Style::ApplySASS("QTextEdit "
    "{"
    " font: @RegularTextFont;"
    " color: @uiInputElementColorText;"
    "} "
    "QTextEdit[plainText=\"\"] "
    "{"
    " font: @RegularTextFont;"
    " color: @uiInputElementColorTextDisabled;"
    "}"));
  auto f = QFont();
  f.setFamily(Style::GetSASSValue("fontFaceNormal"));
  f.setWeight(QFont::Weight::Normal);
  f.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
  ui->commentEdit->setFont(f);
  ui->commentEdit->style()->polish(ui->commentEdit);

  connect(ui->commentEdit, &QTextEdit::textChanged, this, [this]()
  {
    ui->commentEdit->style()->unpolish(ui->commentEdit);
    ui->commentCountLabel->setText(QString("%1/%2").arg(ui->commentEdit->toPlainText().length()).arg(mSettings.textMaxLen));

    if (ui->commentEdit->toPlainText().length() > mSettings.textMaxLen)
    {
      int diff = ui->commentEdit->toPlainText().length() - mSettings.textMaxLen;
      auto prevCursorPos = ui->commentEdit->textCursor().position();

      if (prevCursorPos > mSettings.textMaxLen) prevCursorPos = mSettings.textMaxLen;

      QString newStr = ui->commentEdit->toPlainText();
      newStr.chop(diff);
      ui->commentEdit->setText(newStr);
      QTextCursor cursor(ui->commentEdit->textCursor());
      cursor.setPosition(prevCursorPos, QTextCursor::MoveAnchor);
      ui->commentEdit->setTextCursor(cursor);
      ui->commentEdit->setOverwriteMode(true);
    }
    else if (ui->commentEdit->toPlainText().length() < mSettings.textMaxLen)
    {
      ui->commentEdit->setOverwriteMode(false);
    }//if equals, do nothing
  });
}
