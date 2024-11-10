#pragma once

#include <QLayout>
#include <QStyle>
#include <QMap>

class FlowLayout : public QLayout
{
  Q_OBJECT

public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    void insertWidget(qsizetype ind, QWidget* w);
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    void setSpacing(int spacing) override;
    int rows() const;
    int getItemCountForRow(int row);
signals:
  void geometryUpdated();

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    mutable QMap<int, int> mRowItemCounts;
    int m_hSpace;
    int m_vSpace;
};
