#ifndef WEB_DIALOG_IMPORT_XLS_FILE_H
#define WEB_DIALOG_IMPORT_XLS_FILE_H

#include <QComboBox>
#include "ui/dialogs/templates/info.h"
#include "ui/controls/dnd_zone_frame.h"
#include "ui/controls/file_plate_widget.h"
#include "logic/enums.h"
#include <ui/dialogs/web_overlay.h>

namespace Ui {
class WebDialogImportXlsFile;
}

struct StringKey
{
  QString translation;
  QString key;
  QString context;
  bool operator==(const StringKey& other)const
  {
    if(key == other.key)
      return true;
    return false;
  }
  QString toString() const
  {
    return translation;
  }
};
Q_DECLARE_METATYPE(StringKey);

struct LibElem
{
  QString fullTitle;
  QString shortTitle;
  int libraryElementId = -1;
  int libraryGroupId = -1;
  int type = 0;// 1 = compound, 2 = coefficient
  QString toString() const
  {
    return shortTitle;
  }
};
Q_DECLARE_METATYPE(LibElem);

class StringKeyEditor : public QComboBox
{
  Q_OBJECT
public:
  StringKeyEditor(QWidget* parent = nullptr)
    :QComboBox(parent)
  {
    setFrame(false);
    connect(this, &QComboBox::currentIndexChanged, this,
            [this](int index)
            {
              emit currentStringKeyChanged(currentData().value<StringKey>());
            });
    setSizeAdjustPolicy(AdjustToMinimumContentsLengthWithIcon);
  }

  StringKey currentStringKey() const
  {
    return currentData().value<StringKey>();
  }
  void setCurrentStringKey(const StringKey &newCurrentStringKey)
  {
    auto currentValue = currentData().value<StringKey>();
    if (!currentValue.key.isEmpty() && currentValue.key == newCurrentStringKey.key)
      return;
    auto currentKeys = mStringKeysContexts.value(newCurrentStringKey.context, {});
    int maxSymCount = 0;
    for(auto& key : currentKeys)
    {
      if(key.translation.size() > maxSymCount)
        maxSymCount = key.translation.size();
      addItem(key.translation, QVariant::fromValue(key));
      if(key.key == newCurrentStringKey.key)
        setCurrentIndex(count() - 1);
    }
    setMinimumContentsLength(maxSymCount);
    setMinimumWidth(minimumSizeHint().width());
  }

  static void initStringKeyContextByModule(WebOverlay *overlay = nullptr);
  static const QMap<QString, QList<StringKey>>& keysContexts(){return mStringKeysContexts;};
signals:
  void currentStringKeyChanged(StringKey key);
  void keysLoaded();

private:
  static QMap<QString, QList<StringKey>> mStringKeysContexts;
  Q_PROPERTY(StringKey currentStringKey READ currentStringKey WRITE setCurrentStringKey NOTIFY currentStringKeyChanged USER true)
};

class LibElemEditor : public QComboBox
{
  Q_OBJECT
public:
  LibElemEditor(QWidget* parent = nullptr)
    :QComboBox(parent)
  {
    setFrame(false);
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    setSizeAdjustPolicy(AdjustToMinimumContentsLengthWithIcon);
  }

  LibElem currentElem() const
  {
    if(currentIndex() == -1)
    {
      return model()->index(0,0).data().value<LibElem>();
    }
    return currentData().value<LibElem>();
  }
  void setCurrentElem(const LibElem &newCurrentElem)
  {
    auto currentValue = currentData().value<LibElem>();
    if (currentValue.libraryElementId != -1 && currentValue.libraryElementId == newCurrentElem.libraryElementId)
      return;

    int maxSymCount = 0;
    for(auto& compound : (newCurrentElem.type == 1 ? mCompounds : mCoefficients))
    {
      if(maxSymCount < compound.shortTitle.size())
        maxSymCount = compound.shortTitle.size();
      addItem(compound.shortTitle, QVariant::fromValue(compound));
      if(compound.libraryElementId == newCurrentElem.libraryElementId)
        setCurrentIndex(count() - 1);
    }
    setMinimumContentsLength(maxSymCount);
    setMinimumWidth(minimumSizeHint().width());
  }

  static const QList<LibElem>& compounds() {return mCompounds;};
  static const QList<LibElem>& coefficients() {return mCoefficients;};

  static void setCompounds(const QList<LibElem>& compounds)
  {
    mCompounds = compounds;
  }
  static void setCoefficients(const QList<LibElem>& coefficients)
  {
    mCoefficients = coefficients;
  }

private:
  static QList<LibElem> mCoefficients;
  static QList<LibElem> mCompounds;
  Q_PROPERTY(LibElem currentElement READ currentElem WRITE setCurrentElem USER true)
};

class QMenu;
class ImportXlsProxyModel;

namespace Dialogs
{
using FilePlatesMap = QMap<QString, FilePlateWidget*>;

class WebDialogImportXlsFile : public Templates::Info
{
  Q_OBJECT

public:
  enum Page
  {
    PageFilesUpload = 0,
    PagePreviewTable = 1,
    PageTitleCommentSetup = 2,
    PageChromatogrammsGroupSetup = 3,

    PageLast,
  };

private:
  QStringList kStepsTips;
  QString kInterstageOkButtonText;
  QString kFinalOkButtonText;

public:
  explicit WebDialogImportXlsFile(int projectId, QWidget* parent = nullptr);
  ~WebDialogImportXlsFile();

public slots:
  void addFilesToList(FileInfoList infoList);

  void Accept() override;
  void Reject() override;

private slots:
  void onPlateClosed(QString absoluteFilePath);
  void stepForward();
  void stepBack();
  void inputValidityCheck(std::optional<bool> forceValidState = {});
  void loadTablePreview();
  void importFromLocal();
  void onColumnsModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QList<int> &roles = QList<int>());
  void onTableContextMenuRequested(const QPoint &pt);
  void hideRow(int row = -1);
  void hideColumn(int column = -1);
  void restoreValue();

private:
  Ui::WebDialogImportXlsFile* ui;
  Page                        mCurrentPage = PageFilesUpload;
  FilePlatesMap               mFilePlates;
  QPointer<QPushButton>       mStepBackBtn;
  QMetaObject::Connection     mColumnsTableConnection;
  QPointer<ImportXlsProxyModel> mTableProxy;
  QPointer<QMenu> mTableMenu;
  QPointer<QMenu> mVHeaderMenu;
  QPointer<QMenu> mHHeaderMenu;
  QPointer<QAction> mRemoveColumnAction;
  QPointer<QAction> mRemoveRowAction;
  QPointer<QAction> mRestoreRowAction;
  QPointer<QAction> mRestoreColumnAction;
  QPointer<QAction> mRestoreValueAction;

  int mProjectId{};
  QString mTableName;

  void setupUi();
  void uploadTable();
};
}
#endif // WEB_DIALOG_IMPORT_XLS_FILE_H
