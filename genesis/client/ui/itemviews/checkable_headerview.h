#ifndef DATATABLEHEADERVIEW_H
#define DATATABLEHEADERVIEW_H

#include <QHeaderView>
#include <set>

class CheckableHeaderView : public QHeaderView
{
  Q_OBJECT
public:
  CheckableHeaderView(Qt::Orientation orientation, QWidget * parent = nullptr);
  void SetNonCheckableSections(const QSet<int>& sections);
  void SetSectionCheckable(int section, bool checkable);
  QSet<int> CheckedSections() const;
  QSet<int> UnCheckedSections() const;
  QSet<int> CheckableSections() const;
  void SetSectionChecked(int section, bool checked);
  void SetSectionsChecked(const QList<QPair<int, bool> > &sectionsStates);
  void SetSectionsChecked(const QList<int>& sections, bool isChecked);
  void SetSectionsChecked(const QSet<int>& sections, bool isChecked);
  void SetSectionsCheckedByIdRole(const QList<int>& sections, bool isChecked);
  std::set<std::pair<int, bool>> SectionsCheckStates() const;
  void SetAllCheckState(bool bChecked);
  void SetCheckboxesHidden(bool isHidden);

protected:
  void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
  QSize sectionSizeFromContents(int logicalIndex) const override;
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void setModel(QAbstractItemModel *model) override;
  QStyleOptionButton calcCheckboxOpt(const QRect& rect, int logicalIndex) const;
  void defaultPaintWithLabelOffset(QPainter *painter, const QRect &rect, int logicalIndex, QPoint labelTranslation) const;

signals:
  void checkStateChanged(const std::set<std::pair<int, bool>>&);

protected:
  QRect CheckBoxLocalRect;
  int platesMargins;
  bool isCheckBoxesHidden;

private:
  //returns true if any checkbox was hitted
  bool handleCheckBoxClick(const QPoint& pos);
  void onCheckStateChanged();
  void updateItemTextColors();

  int CheckBoxHovered;

};

#endif // DATATABLEHEADERVIEW_H
