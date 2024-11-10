#pragma once

#include <QPlainTextEdit>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
  Q_OBJECT

public:
  explicit CodeEditor(QWidget* parent);
  ~CodeEditor();

protected:
  void resizeEvent(QResizeEvent *event) override;

  void LineNumberAreaPaintEvent(QPaintEvent *event);
  int LineNumberAreaWidth();

private slots:
  void UpdateLineNumberAreaWidth(int newBlockCount);
  void UpdateLineNumberArea(const QRect &rect, int dy);

private:
  LineNumberArea* NumberArea;
  friend class LineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
  explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), Editor(editor)
  {}

  QSize sizeHint() const override
  {
    return QSize(Editor->LineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override
  {
    Editor->LineNumberAreaPaintEvent(event);
  }

private:
  CodeEditor* Editor;
};
