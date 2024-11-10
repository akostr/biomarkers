#ifndef VIEWPAGECHROMATOGRAMMSFRAGMENTS_H
#define VIEWPAGECHROMATOGRAMMSFRAGMENTS_H

#include <QNetworkReply>
#include "view.h"

class QPushButton;
class QTableView;
class ChromatogrammsTableWidget;
class FragmentsTableWidget;
namespace Views
{
class ViewPageChromatogrammsFragments : public View
{
  Q_OBJECT
public:
  ViewPageChromatogrammsFragments(QWidget* parent = nullptr);
  virtual ~ViewPageChromatogrammsFragments() = default;
  virtual void ApplyContextProject(const QString& dataId, const QVariant& data) override;
  virtual void ApplyContextModules(const QString& dataId, const QVariant& data) override;
  void setupUi();

public slots:
  void importFiles();

private:
  QPointer<QPushButton>                   m_importFilesButton;
  QPointer<ChromatogrammsTableWidget>     m_chromatogrammsWidget;
  QPointer<FragmentsTableWidget>          m_fragmentsWidget;
  int m_projectId;
};

#endif // VIEWPAGECHROMATOGRAMMSFRAGMENTS_H
}//namespace Views
