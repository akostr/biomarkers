#ifndef DIALOG_TEXT_EDIT_EX_H
#define DIALOG_TEXT_EDIT_EX_H

#include <QWidget>

namespace Ui {
class DialogTextEditEx;
}

class DialogTextEditEx : public QWidget
{
  Q_OBJECT

public:
  struct Settings
  {
    QString textHeader = tr("Comment title");
    bool textRequired = false;
    QString textPlaceholder = tr("Enter comment info here");
    int textMaxLen = 70;
    QString defaultText;
  };

  explicit DialogTextEditEx(QWidget* parent = nullptr);
  ~DialogTextEditEx();

  void applySettings(const Settings& settings);
  Settings settings() const;

  QString text();

private:
  Ui::DialogTextEditEx* ui;
  Settings mSettings;

private:
  void setupUi();
};

#endif // DIALOG_TEXT_EDIT_EX_H
