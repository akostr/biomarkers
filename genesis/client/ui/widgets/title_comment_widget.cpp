#include "title_comment_widget.h"
#include "ui_title_comment_widget.h"

#include "genesis_style/style.h"

#include <QStyle>

namespace
{
  const int MaxTitleLength = 50;
  const int MaxCommentLength = 70;
  const std::string_view t_style = "style";
  const std::string_view t_error = "error";
  const std::string_view t_success = "success";
}

namespace Widgets
{
  TitleCommentWidget::TitleCommentWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::TitleCommentWidget())
    , ValidIcon("://resource/icons/icon_success_on@2x.png")
    , InvalidIcon("://resource/icons/icon_error@2x.png")
    , UncheckedIcon("://resource/icons/icon_success_off@2x.png")
  {
    SetupUi();
    ConnectSignals();
  }

  TitleCommentWidget::~TitleCommentWidget()
  {
    delete ui;
  }

  QString TitleCommentWidget::Title() const
  {
    return ui->titleLineEdit->text();
  }

  QString TitleCommentWidget::Comment() const
  {
    return ui->commentTextEdit->toPlainText();
  }

  void TitleCommentWidget::SetTitle(const QString& title)
  {
    ui->titleLineEdit->setText(title);
  }

  void TitleCommentWidget::SetComment(const QString& comment)
  {
    ui->commentTextEdit->setText(comment);
  }

  void TitleCommentWidget::SetForbiddenTitles(QStringList forbiddenTitles)
  {
    forbiddenTitlesSet = {forbiddenTitles.begin(), forbiddenTitles.end()};
  }

  bool TitleCommentWidget::IsValid()
  {
    auto text = ui->titleLineEdit->text();
    bool forbidden = forbiddenTitlesSet.contains(text);
    return !text.isEmpty() && !forbidden;
  }

  void TitleCommentWidget::SetupUi()
  {
    ui->setupUi(this);
    setStyleSheet(Style::Genesis::GetUiStyle());
    const auto rl = Style::Genesis::Fonts::RegularLight();
    ui->titleLabel->setStyleSheet(rl);
    ui->titleLineEdit->setMaxLength(MaxTitleLength);
    ui->commentLabel->setStyleSheet(rl);
    ui->statusCommentLabel->setStyleSheet(rl);
    ui->commentTextEdit->setStyleSheet(Style::ApplySASS("QTextEdit "
      "{"
      " font: @RegularTextFont;"
      " color: @uiInputElementColorText;"
      "} "
      "QTextEdit[plainText=\"\"] "
      "{"
      " font: @RegularTextFont;"
      " color: @uiInputElementColorTextDisabled;"
      "}"));
    ui->titleLineEdit->setStyleSheet(Style::ApplySASS("QLineEdit {color:@uiInputElementColorText;} QLineEdit[text=\"\"] {color:@uiInputElementColorTextDisabled;}\n"
      "QLineEdit[style=\"error\"] {border-color: @errorColor;}\n"
      "QLineEdit[style=\"success\"] {border-color: @successColor;}\n"
      "QLineEdit:disabled {border-color: lightgray;}"));
    ui->statusTitleLabel->setStyleSheet(Style::ApplySASS("QToolTip { color: #ffffff; background-color: black; border-radius: 4px;}\n"
      "QLabel[style=\"error\"] QToolTip {background-color: @errorColor;}\n"
      "QLabel[style=\"success\"] QToolTip {background-color: @successColor;}\n"
      "QLabel:disabled QToolTip{background-color: lightgray;}"));

    auto f = QFont();
    f.setFamily(Style::GetSASSValue("fontFaceNormal"));
    f.setWeight(QFont::Weight::Normal);
    f.setPixelSize(Style::GetSASSValue("fontSizeRegularTextScalableFont").toInt());
    ui->commentTextEdit->setFont(f);
    ui->commentTextEdit->style()->polish(ui->commentTextEdit);
    ui->titleIconLabel->setPixmap(InvalidIcon);
    ui->titleLabel->setText(ui->titleLabel->text() + Style::GetInputAlert());
    onTitleTextChanged();
    onCommentTextChanged();
  }

  void TitleCommentWidget::ConnectSignals()
  {
    connect(ui->titleLineEdit, &QLineEdit::textChanged, this, &TitleCommentWidget::onTitleTextChanged);
    connect(ui->titleLineEdit, &QLineEdit::textChanged, this, &TitleCommentWidget::titleTextChanged);
    connect(ui->commentTextEdit, &QTextEdit::textChanged, this, &TitleCommentWidget::onCommentTextChanged);
  }

  void TitleCommentWidget::onCommentTextChanged()
  {
    ui->statusCommentLabel->setText(QString("%1/%2").arg(ui->commentTextEdit->toPlainText().length()).arg(MaxCommentLength));
    ui->commentTextEdit->style()->unpolish(ui->commentTextEdit);
    const auto commentLength = ui->commentTextEdit->toPlainText().length();
    if (commentLength > MaxCommentLength)
    {
      QString newStr = ui->commentTextEdit->toPlainText();
      newStr.chop(commentLength - MaxCommentLength);
      ui->commentTextEdit->setText(newStr);
      QTextCursor cursor(ui->commentTextEdit->textCursor());
      cursor.setPosition(std::min(ui->commentTextEdit->textCursor().position(), MaxCommentLength), QTextCursor::MoveAnchor);
      ui->commentTextEdit->setTextCursor(cursor);
      ui->commentTextEdit->setOverwriteMode(true);
    }
    if (commentLength < MaxCommentLength)
    {
      ui->commentTextEdit->setOverwriteMode(false);
    }//if equals, do nothing
  }

  void TitleCommentWidget::onTitleTextChanged()
  {
    ui->statusTitleLabel->setText(QString("%1/%2").arg(ui->titleLineEdit->text().length()).arg(MaxTitleLength));
    ui->titleIconLabel->setPixmap(!IsValid() ? InvalidIcon : ValidIcon);
    ui->titleIconLabel->setProperty(t_style.data(), !IsValid() ? t_error.data() : t_success.data());
    ui->titleLineEdit->setProperty(t_style.data(), !IsValid()? t_error.data() : t_success.data());
    ui->titleIconLabel->style()->polish(ui->titleIconLabel);
    ui->titleLineEdit->style()->polish(ui->titleLineEdit);
  }
}
