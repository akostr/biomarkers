#include "dnd_files.h"

#include "../../genesis_style/style.h"

#include <QScrollBar>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>

namespace
{
  class PlainScrollArea : public QScrollArea
  {
  public:
    explicit PlainScrollArea(QWidget* parent = nullptr)
      : QScrollArea(parent)
    {
    }

    QSize sizeHint() const
    {
      int f = 2 * frameWidth();
      QSize sz(f, f);
      QWidget* w = widget();
      if (w)
      {
        QSize ws = widgetResizable() ? w->sizeHint() : w->size();
        sz += ws;
      }
      sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width() + 5);
      return sz;
    }
  };
}

////////////////////////////////////////////////////
//// DnD files view
DnDFiles::DnDFiles(QWidget* parent)
  : QFrame(parent)
{
  Setup();
}

DnDFiles::~DnDFiles()
{
}

void DnDFiles::AddFile(const QString& file, const QByteArray& data)
{
  if (file.isEmpty() || data.isEmpty())
    return;

  if (!Files.contains(file))
  {
    Files[file] = data;
    
    //// Name
    QFileInfo info(file);
    FileInfos[file].DisplayName = info.fileName();

    //// State
    if (UnacceptableFiles.contains(file))
      FileInfos[file].State = FileInfo::StateError;

    emit FilesChanged();
  }
}

//// Remove file
void DnDFiles::RemoveFile(const QString& file)
{
  if (Files.contains(file))
  {
    Files.remove(file);
    FileInfos.remove(file);

    emit FilesChanged();
  }
}

//// Clear
void DnDFiles::Clear()
{
  Files.clear();
  FileInfos.clear();

  emit FilesChanged();
}

QStringList DnDFiles::GetFiles() const
{
  return Files.keys();
}

QMap<QString, QByteArray>& DnDFiles::GetFilesData() const
{
  return const_cast<QMap<QString, QByteArray>&>(Files);
}

//// Count
int DnDFiles::GetFilesCount() const
{
  return Files.size();
}

//// Has errors
bool DnDFiles::HasErrors() const
{
  for (auto& fileInfo : FileInfos)
  {
    if (fileInfo.State == FileInfo::StateError)
    {
      return true;
    }
  }
  return false;
}

QStringList DnDFiles::GetAcceptableFiles(const QList<QUrl>& urls) const
{
  QStringList files;

  //// Sanitize
  for (auto& url : urls)
  {
    QString path = url.toLocalFile();
    QFileInfo info(path);
    if (info.isFile() && (info.suffix().toLower() == "cdf"))
    {
      files << path;
    }
  }

  return files;
}

void DnDFiles::SetUnacceptableFiles(const QStringList& files)
{
  UnacceptableFiles = QSet<QString>(files.begin(), files.end());
  UpdateFileStates();
}

void DnDFiles::UpdateFileStates()
{
  bool update = false;
  for (auto& f : Files.keys())
  {
    if (UnacceptableFiles.contains(f))
    {
      if (FileInfos[f].State != FileInfo::StateError)
      {
        FileInfos[f].State = FileInfo::StateError;
        update = true;
      }
    }
    else
    {
      if (FileInfos[f].State == FileInfo::StateError)
      {
        FileInfos[f].State = FileInfo::StateNormal;
        update = true;
      }
    }
  }

  if (update)
  {
    emit FilesChanged();
  }
}

void DnDFiles::dragEnterEvent(QDragEnterEvent* event)
{
  if (GetAcceptableFiles(event->mimeData()->urls()).isEmpty())
    event->ignore();
  else
    event->acceptProposedAction();
}

void DnDFiles::dragMoveEvent(QDragMoveEvent* event)
{
  event->acceptProposedAction();
}

void DnDFiles::dragLeaveEvent(QDragLeaveEvent* event)
{
  event->accept();
}

void DnDFiles::dropEvent(QDropEvent* event)
{
  //// Get uris
  QStringList files = GetAcceptableFiles(event->mimeData()->urls());

  //// Read in place
  for (auto& fileName : files)
  {
    //// Read
    QByteArray fileContent;
    if (!fileName.isEmpty())
    {
      QFile file(fileName);
      file.open(QIODevice::ReadOnly);
      fileContent = file.readAll();
    }

    //// Apply
    AddFile(fileName, fileContent);
  }
}

void DnDFiles::Setup()
{
  //// Self
  setAcceptDrops(true);

  //// Layout
  Layout = new QVBoxLayout(this);
  Layout->setContentsMargins(0, 0, 0, 0);

  //// Scroll
  {
    ScrollArea = new QScrollArea(this);
    Layout->addWidget(ScrollArea);

    ScrollArea->setStyleSheet("QWidget { background-color:white; }");

    //// Scroll area content
    {
      ScrollAreaContent = new QWidget(ScrollArea);
      
      ScrollArea->setFrameShape(QFrame::NoFrame);
      ScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      ScrollArea->setAutoFillBackground(false);
      ScrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
      ScrollArea->setWidget(ScrollAreaContent);
      ScrollArea->setWidgetResizable(true);

      ScrollAreaContentLayout = new QVBoxLayout(ScrollAreaContent);
      ScrollAreaContentLayout->setContentsMargins(0, 0, 0, 0);
      ScrollAreaContentLayout->setSpacing(0);

      //// Files
      {
        ScrollAreaContentFiles = new QWidget(ScrollAreaContent);
        ScrollAreaContentLayout->addWidget(ScrollAreaContentFiles);

        ScrollAreaContentFilesLayout = new QVBoxLayout(ScrollAreaContentFiles);
        ScrollAreaContentFilesLayout->setContentsMargins(0, 0, 0, 0);

        //// View
        {
          ScrollAreaContentFilesView = new QLabel(ScrollAreaContentFiles);
          ScrollAreaContentFilesLayout->addWidget(ScrollAreaContentFilesView);

          //// Handle commands
          connect(ScrollAreaContentFilesView, &QLabel::linkActivated, [this](const QString& link)
            {
              QString command = link;
              QString commandRemove = "#remove#";
              if (command.startsWith(commandRemove))
              {
                command = command.right(command.length() - commandRemove.length());
                RemoveFile(command);
              }
            });
        }

        //// Stretch
        ScrollAreaContentLayout->addStretch();
      }

      //// Browse controls
      {
        //// Space
        ScrollAreaContentLayout->addSpacing(Style::Scale(16));

        //// Hint
        QLabel* hint = new QLabel(tr("Drag and drop files here or"), ScrollAreaContent);
        ScrollAreaContentLayout->addWidget(hint, 0, Qt::AlignCenter);

        //// Space
        ScrollAreaContentLayout->addSpacing(Style::Scale(16));

        //// Button
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Reset, ScrollAreaContent);
        ScrollAreaContentLayout->addWidget(buttons, 0 ,Qt::AlignHCenter);
        if (QPushButton* b = buttons->button(QDialogButtonBox::Reset))
        {
          auto fileContentReady = [this](const QString& fileName, const QByteArray& fileContent) 
          {
            //// File selected
            if (!fileName.isEmpty()) 
            {
              //// Handle file data
              AddFile(fileName, fileContent);
            }
          };

          b->setText(tr("Browse for file"));
          connect(b, &QAbstractButton::clicked, [this, fileContentReady]()
            {
#ifdef Q_OS_WASM
              //// Web assembly
              QFileDialog::getOpenFileContent(tr("CDF files (*.cdf)"), fileContentReady);
#else // !Q_OS_WASM
              //// Desktop
              QStringList files = QFileDialog::getOpenFileNames(this,
                tr("Upload CDF file"),
                QString(),
                tr("CDF files (*.cdf)"));

              //// Read in place
              for (auto& fileName : files)
              {
                //// Read
                QByteArray fileContent;
                if (!fileName.isEmpty())
                {
                  QFile file(fileName);
                  file.open(QIODevice::ReadOnly);
                  fileContent = file.readAll();
                }

                //// Apply
                fileContentReady(fileName, fileContent);
              }
#endif // ~!Q_OS_WASM
            });
        }
      }

      ScrollAreaContentLayout->addStretch();
    }
  }

  //// Connect
  connect(this, &DnDFiles::FilesChanged, this, &DnDFiles::Update);
}

void DnDFiles::Update()
{
  //// Line template
  QString lineTemplate      = Style::ApplySASS("<table padding=\"2px\"><tr><td valign=\"middle\"><span style=\"color: @textColor;\"     >%1&nbsp;</span></td><td valign=\"middle\">%2</td></tr></table>");
  QString lineTemplateError = Style::ApplySASS("<table padding=\"2px\"><tr><td valign=\"middle\"><span style=\"color: @textColorError;\">%1&nbsp;</span></td><td valign=\"middle\">%2</td></tr></table>");

  //// Document
  QString document;
  for (auto& f : Files.keys())
  {
    //// Removal icon
    QString removalLink = QString("<a href=\"#remove#%1\"><img src=\":/resource/icons/icon_dnd_area_remove.png\"/></a>").arg(f);

    //// Format line
    QString line = (FileInfos[f].State == FileInfo::StateError ? lineTemplateError : lineTemplate)
      .arg(FileInfos[f].DisplayName)
      .arg(removalLink);
    document += line;
  }
  ScrollAreaContentFilesView->setText(document);
}
