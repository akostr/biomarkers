#pragma once

#include <QWidget>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

#include <QUrl>
#include <QPointer>
#include <QMap>
#include <QSet>

////////////////////////////////////////////////////
//// DnD files view
class DnDFiles : public QFrame
{
  Q_OBJECT

public:
  struct FileInfo
  {
    enum States
    {
      StateNormal,
      StateError
    };

    QString DisplayName;
    States  State = StateNormal;
  };

public:
  DnDFiles(QWidget* parent = nullptr);
  ~DnDFiles();

  //// Add file
  void AddFile(const QString& file, const QByteArray& data);

  //// Remove file
  void RemoveFile(const QString& file);

  //// Clear
  void Clear();

  //// Get files
  QStringList GetFiles() const;

  //// Get files data
  QMap<QString, QByteArray>& GetFilesData() const;

  //// Count
  int GetFilesCount() const;

  //// Has errors
  bool HasErrors() const;

  //// Set unacceptable files
  void SetUnacceptableFiles(const QStringList& files);

private:
  //// Get acceptable files
  QStringList GetAcceptableFiles(const QList<QUrl>& urls) const;

  //// Update file states
  void UpdateFileStates();

private:
  //// Dnd impl
  virtual void dragEnterEvent(QDragEnterEvent* event) override;
  virtual void dragMoveEvent(QDragMoveEvent* event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
  virtual void dropEvent(QDropEvent* event) override;

signals:
  void FilesChanged();

private slots:
  void Setup();
  void Update();

private:
  //// Data
  QMap<QString, QByteArray> Files;
  QMap<QString, FileInfo>   FileInfos;
  QSet<QString>             UnacceptableFiles;

  //// Ui
  QPointer<QVBoxLayout>     Layout;
  QPointer<QScrollArea>     ScrollArea;
  QPointer<QWidget>         ScrollAreaContent;
  QPointer<QVBoxLayout>     ScrollAreaContentLayout;
  QPointer<QWidget>         ScrollAreaContentFiles;
  QPointer<QVBoxLayout>     ScrollAreaContentFilesLayout;
  QPointer<QLabel>          ScrollAreaContentFilesView;
};
