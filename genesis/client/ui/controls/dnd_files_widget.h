#ifndef DND_FILES_WIDGET_H
#define DND_FILES_WIDGET_H

#include <QWidget>
#include <QFileInfo>

using FileInfoList = QList<QFileInfo>;
namespace Ui {
class DnDFilesWidget;
}

class DnDFilesWidget : public QWidget
{
  Q_OBJECT

public:
  explicit DnDFilesWidget(QWidget *parent = nullptr);
  ~DnDFilesWidget();
  void setFrameCaption(QString frameCaption);
  void setAcceptableDrops(QStringList extentions);
  void setFilesDialogFilters(QString filters);
  void setFilesDialogCaption(QString purpose);

signals:
  void filesAdded(FileInfoList newFilesList);

private:
  Ui::DnDFilesWidget *ui;
  QStringList mAcceptableDrops;
  QString mFdFilters;
  QString mFdCaption;

private:
  void onLocalImportButtonClicked();
};

#endif // DND_FILES_WIDGET_H
