#pragma once
#ifndef TITLE_COMMENT_WIDGET_H
#define TITLE_COMMENT_WIDGET_H

#include <QWidget>

namespace Ui
{
  class TitleCommentWidget;
}

namespace Widgets
{
  class TitleCommentWidget final : public QWidget
  {
    Q_OBJECT
  public:
    explicit TitleCommentWidget(QWidget* parent = nullptr);
    ~TitleCommentWidget();

    QString Title() const;
    QString Comment() const;
    void SetTitle(const QString& title);
    void SetComment(const QString& comment);
    void SetForbiddenTitles(QStringList forbiddenTitles);
    bool IsValid();

  signals:
    void titleTextChanged();

  private:
    Ui::TitleCommentWidget* ui = nullptr;

    QPixmap ValidIcon;
    QPixmap InvalidIcon;
    QPixmap UncheckedIcon;
    QSet<QString> forbiddenTitlesSet;

    void SetupUi();
    void ConnectSignals();

    void onCommentTextChanged();
    void onTitleTextChanged();
  };
}

#endif
