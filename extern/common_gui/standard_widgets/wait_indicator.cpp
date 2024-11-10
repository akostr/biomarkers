#include "wait_indicator.h"

#include <QTimer>
#include <QIcon>

WaitIndicator::WaitIndicator(const QString& text, QWidget* parent, int size)
  : QLabel(text, parent)
  , Size(size)
  , IconIndex(0)
  , AnimationTimer(nullptr)
{
  Initialize();
  setPixmap(Icons[IconIndex].pixmap(QSize(Size, Size)));
}

WaitIndicator::WaitIndicator(QWidget* parent)
  : WaitIndicator(QString(), parent)
{
}

WaitIndicator::~WaitIndicator()
{
}

void WaitIndicator::SetIcons(const QStringList& icons)
{
  Icons.clear();
  Icons.reserve(icons.size());
  for (auto i : icons)
    Icons.append(QIcon(i));

  if (IconIndex >= Icons.size())
    IconIndex = 0;
  setPixmap(Icons[IconIndex].pixmap(QSize(Size, Size)));
}

void WaitIndicator::OnFrameTimeout()
{
  if (!isVisibleTo(window()))
    return;

  ++IconIndex;
  if (IconIndex >= Icons.size())
    IconIndex = 0;
  setPixmap(Icons[IconIndex].pixmap(QSize(Size, Size)));
}

void WaitIndicator::Initialize()
{
  AnimationTimer = new QTimer(this);
  connect(AnimationTimer, SIGNAL(timeout()), this, SLOT(OnFrameTimeout()));
  AnimationTimer->start(60);

  SetIcons({
     ":/resource/controls/loading0.png",
     ":/resource/controls/loading30.png",
     ":/resource/controls/loading60.png",
     ":/resource/controls/loading90.png",
     ":/resource/controls/loading120.png",
     ":/resource/controls/loading150.png",
     ":/resource/controls/loading180.png",
     ":/resource/controls/loading210.png",
     ":/resource/controls/loading240.png",
     ":/resource/controls/loading270.png",
     ":/resource/controls/loading300.png",
     ":/resource/controls/loading330.png",
   });
}
