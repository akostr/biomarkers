#ifndef WEB_DIALOG_CHECK_CODE_WORD_H
#define WEB_DIALOG_CHECK_CODE_WORD_H

#include <ui/dialogs/templates/confirm.h>

namespace Ui {
class WebDialogCheckCodeWord;
}

namespace Dialogs
{
namespace Templates
{
class WebDialogCheckCodeWord : public Confirm
{
  Q_OBJECT

public:
  explicit WebDialogCheckCodeWord(const QString& codeWord, QWidget* parent = nullptr);
  ~WebDialogCheckCodeWord();

  void setupUi();

private:
  Ui::WebDialogCheckCodeWord* ui;
  QWidget* mContent;

  QString mCodeWord;
};
}
}

#endif // WEB_DIALOG_CHECK_CODE_WORD_H
