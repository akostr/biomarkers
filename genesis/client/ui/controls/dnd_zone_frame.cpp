#include "dnd_zone_frame.h"
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QStyle>

DnDZoneFrame::DnDZoneFrame(QWidget *parent)
  : QFrame(parent)
{

}

void DnDZoneFrame::dragEnterEvent(QDragEnterEvent *event)
{
  if (getAcceptableFiles(event->mimeData()->urls()).isEmpty())
    event->ignore();
  else
  {
    event->acceptProposedAction();
    setProperty("hover", true);
    style()->polish(this);
  }
  }

void DnDZoneFrame::dragMoveEvent(QDragMoveEvent *event)
{
  event->acceptProposedAction();
}

void DnDZoneFrame::dragLeaveEvent(QDragLeaveEvent *event)
{
  event->accept();
  if(property("hover").toBool())
  {
    setProperty("hover", false);
    style()->polish(this);
  }
}

void DnDZoneFrame::dropEvent(QDropEvent *event)
{
  //// Get uris
  FileInfoList fileInfos = getAcceptableFiles(event->mimeData()->urls());
  if(!fileInfos.empty())
    emit filesDropped(fileInfos);

  if(property("hover").toBool())
  {
    setProperty("hover", false);
    style()->polish(this);
  }
}

FileInfoList DnDZoneFrame::getAcceptableFiles(const QList<QUrl> &urls) const
{
  FileInfoList files;

  //// Sanitize
  for (auto& url : urls)
  {
    QString path = url.toLocalFile();
    QFileInfo info(path);
    if (info.isFile() && (mAcceptableFileExtensions.isEmpty() || mAcceptableFileExtensions.contains(info.suffix(), Qt::CaseInsensitive)))
    {
      files << info;
    }
  }

  return files;
}

void DnDZoneFrame::setAcceptableFileExtensions(const QStringList& extensions)
{
  mAcceptableFileExtensions = extensions;
}
