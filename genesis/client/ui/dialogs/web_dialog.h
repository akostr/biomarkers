#pragma once

#include <QFrame>
#include <QDialogButtonBox>
#include <QPointer>
#include <functional>

class QVBoxLayout;
class QHBoxLayout;


/*
 * если передать в конструктор caller, то для caller и всех его parentWidget будет
 * вызываться функция onDialogRised и onDialogDestroyed, если у caller или его
 * parentWidget-ов такая функция будет определена.
 * Сейчас это используется в разметке, что бы при отображении диалога блокировались
 * Action-ы undo и redo.
*/

/////////////////////////////////////////////////////
//// Web Dialog base class
class WebDialog : public QFrame
{
  Q_OBJECT

public:
  WebDialog(QWidget* caller, QWidget* content = nullptr, QSizeF size = QSizeF(),
            QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok);
  ~WebDialog();

  void SetButtonName(QDialogButtonBox::StandardButton button, const QString& name);
  void SetButtonEnabled(QDialogButtonBox::StandardButton button, bool enabled);
  void SetButtonVisible(QDialogButtonBox::StandardButton button, bool isVisible);

  int GetResult();

  static WebDialog* Question(QString question, QWidget *parent);
  QDialogButtonBox* GetButtonBox() { return ButtonBox; };
  QHBoxLayout* GetButtonBoxLayout() { return ButtonLayout; };

public slots:
  virtual void Open();

  virtual void Accept();
  virtual void Done(int r);
  virtual void Reject();

signals:
  void Accepted();
  void Finished(int result);
  void Rejected();
  void Clicked(QDialogButtonBox::StandardButton button);

public:
  bool eventFilter(QObject* watched, QEvent* event) override;
  bool event(QEvent* event) override;

protected:
  void UpdateGeometry();

protected:
  QPointer<QWidget> Overlay;
  QSizeF            Size;
  int               Result;

  QVBoxLayout*      MainLayout;
  QWidget*          Content;
  QHBoxLayout*      ButtonLayout;
  QDialogButtonBox* ButtonBox;
  QWidget*          Caller;
};

/////////////////////////////////////////////////////
//// Simple dialog box
class WebDialogBox : public WebDialog
{
  Q_OBJECT
public:
  WebDialogBox(const QString& caption, const QString& massage, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok|QDialogButtonBox::Cancel,
      std::function<void()> handle = std::function<void()>());
  ~WebDialogBox();

private:
  QPointer<QWidget>     Body;
  QPointer<QVBoxLayout> BodyLayout;
};
