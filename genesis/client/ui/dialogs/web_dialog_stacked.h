
#ifndef WEBDIALOGSTACKED_H
#define WEBDIALOGSTACKED_H

#include <QFrame>
#include <QStackedLayout>
#include <QPointer>


class WebDialogStacked: public QFrame
{
    Q_OBJECT
public:
    WebDialogStacked(QWidget* parent = nullptr);
    ~WebDialogStacked();

public slots:
    virtual void open();
    virtual void accept();
    virtual void reject();

signals:
    void accepted();
protected:
    QPointer<QLayout>           mMainLayout;
    QPointer<QWidget>           mContent;
    QPointer<QLayout>           mContentLayout;
    QPointer<QStackedLayout>    mStackedLayout;

    QPointer<QWidget>           Overlay;
    QSizeF                      Size;
protected:
    void UpdateGeometry();
    bool eventFilter(QObject* watched, QEvent* event);
};



#endif // WEBDIALOGSTACKED_H
