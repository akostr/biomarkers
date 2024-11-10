#ifndef DNDZONEFRAME_H
#define DNDZONEFRAME_H

#include <QFrame>
#include <QSet>
#include <QFileInfo>

using FileInfoList = QList<QFileInfo>;
class DnDZoneFrame : public QFrame
{
  Q_OBJECT
public:
  DnDZoneFrame(QWidget *parent = nullptr);
  void setAcceptableFileExtensions(const QStringList &extensions);

signals:
  void filesDropped(FileInfoList newFilesList);

private:
  //// Dnd impl
  virtual void dragEnterEvent(QDragEnterEvent* event) override;
  virtual void dragMoveEvent(QDragMoveEvent* event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
  virtual void dropEvent(QDropEvent* event) override;
  FileInfoList getAcceptableFiles(const QList<QUrl>& urls) const;

  QStringList mAcceptableFileExtensions = {"cdf"};
};

#endif // DNDZONEFRAME_H
