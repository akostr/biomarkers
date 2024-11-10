#include "ui/genesis_window.h"
#include "web_dialog.h"
#include "web_overlay.h"

#include <genesis_style/style.h>

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QEvent>

/*
 * если передать сюда caller, то для caller и всех его parentWidget будет вызываться
 * функция onDialogRised и onDialogDestroyed, если у caller или его parentWidget-ов
 * такая функция будет определена.
 * Сейчас это используется в разметке, что бы при отображении диалога блокировались
 * Action-ы undo и redo.
*/

using namespace Core;
/////////////////////////////////////////////////////
//// Web Dialog base class
WebDialog::WebDialog(QWidget* caller
                   , QWidget* content
                   , QSizeF size
                   , QDialogButtonBox::StandardButtons buttons)
  : QFrame(GenesisWindow::Get())
  , Size(size)
  , Result(0)
  , Content(content)
  , Caller(caller)
{
//     QWidget::setAttribute(Qt::WA_DeleteOnClose);
  if (auto w = GenesisWindow::Get())
    w->installEventFilter(this);

  //// Setup
  {
    //// Main layout
    MainLayout = new QVBoxLayout(this);
    MainLayout->setContentsMargins(0, 0, 0, 0);
    MainLayout->setSpacing(0);

    //// Main layout
    {
      //// Content
      if (!Content)
      {
        Content = new QWidget(this);
        Content->setStyleSheet(Style::Genesis::GetUiStyle());

        QVBoxLayout* layout = new QVBoxLayout(Content);
        layout->setContentsMargins(0, 0, 0, 0);
      }

      MainLayout->addWidget(Content, 1);

      //// Separator
//      if (0)
      {
        QFrame* separator = new QFrame(this);
        separator->setFrameShape(QFrame::HLine);
        separator->setObjectName("hline");
        separator->setStyleSheet(Style::Genesis::GetUiStyle());
        MainLayout->addWidget(separator);
      }

      //// Button layout
      {
        ButtonLayout = new QHBoxLayout();
        ButtonLayout->setContentsMargins(Style::Scale(43), Style::Scale(28), Style::Scale(43), Style::Scale(28));

        ButtonBox = new QDialogButtonBox(buttons, this);
        ButtonBox->setStyleSheet(Style::Genesis::GetUiStyle());
        ButtonBox->layout()->setSpacing(20);

        ButtonLayout->addStretch(1);
        ButtonLayout->addWidget(ButtonBox);
//        ButtonLayout->addStretch(1);

        connect(ButtonBox, &QDialogButtonBox::accepted, this, &WebDialog::Accept);
        connect(ButtonBox, &QDialogButtonBox::rejected, this, &WebDialog::Reject);
        connect(ButtonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* button)
          {
            emit Clicked(ButtonBox->standardButton(button));
          });

        MainLayout->addLayout(ButtonLayout, 0);
      }
    }
  }

  //// Self
  setObjectName("WebDialog");
  setStyleSheet("QFrame#WebDialog { border: 1px solid #ecf1f4; background-color: white; border-radius: 8px;}");
  setFrameShape(QFrame::Box);
  setLineWidth(2);

  Size.setWidth(0.3);
  UpdateGeometry();
}

WebDialog::~WebDialog()
{
  if (Overlay)
    delete Overlay;
}

void WebDialog::SetButtonName(QDialogButtonBox::StandardButton button, const QString& name)
{
  if (auto b = ButtonBox->button(button))
    b->setText(name);
}

void WebDialog::SetButtonEnabled(QDialogButtonBox::StandardButton button, bool enabled)
{
  auto btn = ButtonBox->button(button);
  if(btn)
    btn->setEnabled(enabled);
}

int WebDialog::GetResult()
{
  return Result;
}

WebDialog *WebDialog::Question(QString question, QWidget *parent)
{
  auto content = new QWidget();
  auto l = new QVBoxLayout();
  content->setLayout(l);

  l->setAlignment(Qt::AlignCenter);

  auto label = new QLabel(question);
  l->addWidget(label);
  using btns = QDialogButtonBox::StandardButton;
  return new WebDialog(parent, content, QSizeF(), btns::Ok | btns::Cancel);
}

void WebDialog::Open()
{
  qInfo() << " Open dialog: " << metaObject()->className();
  if (!Overlay)
    Overlay = new WebOverlay();
  UpdateGeometry();
  if(Caller)
  {
    auto wgt = Caller;
    while(wgt)
    {
      auto meta = wgt->metaObject();
      meta->invokeMethod(wgt, "onDialogRised");
      wgt = wgt->parentWidget();
    }
  }
  show();
  raise();
}

void WebDialog::Accept()
{
  Done(QDialog::Accepted);
}

void WebDialog::Done(int r)
{
  Result = r;
  hide();
  if (Overlay)
    delete Overlay;

  if (r == QDialog::Accepted)
    emit Accepted();
  else if (r == QDialog::Rejected)
    emit Rejected();
  emit Finished(r);

  if(Caller)
  {
    auto wgt = Caller;
    while(wgt)
    {
      auto meta = wgt->metaObject();
      meta->invokeMethod(wgt, "onDialogDestroyed");
      wgt = wgt->parentWidget();
    }
  }

  deleteLater();
}

void WebDialog::Reject()
{
  Done(QDialog::Rejected);
}

bool WebDialog::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == GenesisWindow::Get() &&
      event->type() == QEvent::Resize)
    UpdateGeometry();
  return QFrame::eventFilter(watched, event);
}

bool WebDialog::event(QEvent* event)
{
  auto ret = QFrame::event(event);
  if (event->type() == QEvent::LayoutRequest)
    UpdateGeometry();
  return ret;
}

void WebDialog::UpdateGeometry()
{
  if (auto window = GenesisWindow::Get())
  {
    double windowW  = window->width();
    double windowhH = window->height();

    double w = windowW  * Size.width();
    double h = windowhH * Size.height();

    QSize hint = sizeHint();
    w = std::max(w, (double)hint.width());
    h = std::max(h, (double)hint.height());

    int x = (windowW - w) / 2;
    int y = (windowhH - h) / 2;

    QRect newGeometry(x, y, w, h);
    if(geometry() != newGeometry)
      setGeometry(x, y, w, h);
  }
}

/////////////////////////////////////////////////////
//// Simple dialog box
WebDialogBox::WebDialogBox(const QString& captionText,
                           const QString& massageText,
                           QDialogButtonBox::StandardButtons buttons,
                           std::function<void()> handle)
  : WebDialog(nullptr, nullptr, QSizeF(), buttons)
{
  //// Body
  Body = new QWidget(Content);
  Content->layout()->addWidget(Body);

  BodyLayout = new QVBoxLayout(Body);
  BodyLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
  BodyLayout->setSpacing(0);

  //// Content
  {
    //// Header
    {
      QLabel* caption = new QLabel(captionText, Body);
      BodyLayout->addWidget(caption);

      caption->setAlignment(Qt::AlignCenter);
      caption->setStyleSheet(Style::Genesis::GetH2());
    }

    //// Space
    BodyLayout->addSpacing(Style::Scale(26));

    //// Text
    {
      QLabel* message = new QLabel(massageText, Body);
      BodyLayout->addWidget(message);
    }
  }

  if (handle)
  {
    connect(this, &WebDialog::Accepted, handle);
  }
}

WebDialogBox::~WebDialogBox()
{
}
