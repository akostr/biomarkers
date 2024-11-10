#include "web_dialog_stacked.h"
#include "../genesis_window.h"
#include <QEvent>
#include "ui/dialogs/web_overlay.h"
#include <genesis_style/style.h>

WebDialogStacked::WebDialogStacked(QWidget *parent)
    : QFrame(GenesisWindow::Get())
    , Size(QSizeF())
{
    setAttribute(Qt::WA_DeleteOnClose);

    mMainLayout = new QVBoxLayout(this);

    mContent = new QWidget();
    mContentLayout = new QVBoxLayout(mContent);
    mContentLayout->setContentsMargins(Style::Scale(40), Style::Scale(32), Style::Scale(40), 0);
    mContentLayout->setSpacing(0);

    mMainLayout->addWidget(mContent);

    mStackedLayout = new QStackedLayout(mContentLayout);
}

WebDialogStacked::~WebDialogStacked()
{
    if (Overlay)
        delete Overlay;
}

void WebDialogStacked::open()
{
    if (!Overlay)
        Overlay = new WebOverlay();
    UpdateGeometry();
    show();
    raise();
}


void WebDialogStacked::accept()
{
    emit accepted();
    close();

}

void WebDialogStacked::reject()
{
    close();
}

void WebDialogStacked::UpdateGeometry()
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

        setGeometry(x, y, w, h);
    }
}

bool WebDialogStacked::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == GenesisWindow::Get())
    {
        if (event->type() == QEvent::Resize)
        {
            UpdateGeometry();
        }
    }
    return false;
}
