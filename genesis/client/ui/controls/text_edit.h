#pragma once

#include <QTextEdit>
#include <QCompleter>
#include <QPointer>

////////////////////////////////////////////////////
/// Text edit
class TextEdit : public QTextEdit
{
  Q_OBJECT

public:
  TextEdit(QWidget *parent = nullptr);
  ~TextEdit();

  //// Completer
  void SetCompleter(QCompleter* completer);
  QCompleter* GetCompleter() const;

  virtual void HandleInput();

protected:
  void keyPressEvent(QKeyEvent* e) override;
  void focusInEvent(QFocusEvent* e) override;

private slots:
  void insertCompletion(const QString &completion);

private:
  QString textUnderCursor() const;

private:
  QPointer<QCompleter> Completer;
};
