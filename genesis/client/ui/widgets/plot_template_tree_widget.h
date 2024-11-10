#ifndef PLOTTEMPLATETREEWIDGET_H
#define PLOTTEMPLATETREEWIDGET_H

#include <QTreeWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class PlotTemplateModel;
class PlotTemplateItem;
class PlotTemplateTreeWidget;

class PlotTemplateTreeItem : public QObject
{
  Q_OBJECT
public:
  PlotTemplateTreeItem(PlotTemplateItem* modelItem, QTreeWidgetItem* widgetItem);

  PlotTemplateItem *modelItem();

public slots:
  void updateIndex();
  void onPropNotify();

private:
  void addTopLevelWidget();
  void addOpacity();
  void addShowOnlyAxesRect();
  void addColor();
  void addFont();
  void addText();

  void addUnifiedColorEditor(const char* propName, const QString& propTitle);
  void addUnifiedCheckbox(const char* propName, const QString& propTitle);
  void addUnifiedSlider(const char* propName, const QString& propTitle, const QString &suffix, int min, int max, bool fromZeroToOneNormalized);
  void addUnifiedSpinBox(const char* propName, const QString& propTitle, const QString &suffix, int min, int max);
  void addUnifiedFont(const char* propName, const QString& propTitle);
  void addUnifiedText(const char* propName, const QString& propTitle);
  void addUnifiedPenStyleCombo(const char* propName, const QString& propTitle);
  void addUnifiedLineEndingStyleCombo(const char* propName, const QString& propTitle);
  void propScan();

  QString trPropName(const char* propName);
  void createEditor(QMetaProperty& prop);
  void removeEditor(const char* propName);
  int getBeforeItemIndex(QString propName);

private:
  PlotTemplateItem *m_modelItem;
  QTreeWidgetItem* m_treeItem;
  static QMap<QString, int> m_propertiesOrder;
};

class PlotTemplateTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  explicit PlotTemplateTreeWidget(QWidget *parent = nullptr);
  void setModel(PlotTemplateModel *model);
  PlotTemplateItem *getModelItem(QTreeWidgetItem*);

private slots:
  void addItem(PlotTemplateItem*);
  void removeItem(PlotTemplateItem*);
  void updateIndexes();
  void updateWidth();

signals:

private:
  PlotTemplateModel *m_model;
  QMap <QTreeWidgetItem*, PlotTemplateTreeItem*> m_items;
};

#endif // PLOTTEMPLATETREEWIDGET_H
