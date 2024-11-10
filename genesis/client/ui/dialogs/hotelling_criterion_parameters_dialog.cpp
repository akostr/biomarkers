#include "hotelling_criterion_parameters_dialog.h"
#include "ui_hotelling_criterion_parameters_dialog.h"
#include <logic/models/analysis_entity_model.h>
#include <ui/flow_layout.h>

#include <QLayout>
#include <QPushButton>

namespace
{
  enum Criteria
{
  none,
  field,
  layer,
  well
};
}

using namespace AnalysisEntity;

HotellingCriterionParametersDialog::HotellingCriterionParametersDialog(QPointer<AnalysisEntityModel> model,
  int step, QWidget* parent) :
  Dialogs::Templates::Info(parent, QDialogButtonBox::Ok | QDialogButtonBox::Cancel)
  , ui(new Ui::HotellingCriterionParametersDialog)
  , mModel(model)
  , mStep(step)
  , mContent(new QWidget(nullptr))
{
  setupUi();
  // ui->spinBox_countOfPC->setMaximum(componentsNumber);

  if(step == 0)
  {
    connect(ui->comboBox_groupingCriterion, &QComboBox::currentIndexChanged, this, &HotellingCriterionParametersDialog::onPCACriterionIngexChanged);

    ui->comboBox_groupingCriterion->addItem(tr("field"), field);
    ui->comboBox_groupingCriterion->addItem(tr("layer"), layer);
    ui->comboBox_groupingCriterion->addItem(tr("well"), well);
  }
}

HotellingCriterionParametersDialog::~HotellingCriterionParametersDialog()
{
  delete ui;
  if(mPcaCache)
  {
    delete mPcaCache;
    delete mPcaExcludedCache;
  }
}

void HotellingCriterionParametersDialog::Accept()
{
  if(mStep == 0)
  {
    if(ui->radioButton_groupingManually->isChecked())
    {
      WebDialog::Accept();
      return;
    }
    mStep = 1;
    setupStep(mStep);
  }
  else
  {
    WebDialog::Accept();
  }
}

int HotellingCriterionParametersDialog::getCountOfPC() const
{
  return ui->spinBox_countOfPC->value();
}

QString HotellingCriterionParametersDialog::getGroupingCriterion() const
{
  return ui->comboBox_groupingCriterion->currentData().toString();
}

bool HotellingCriterionParametersDialog::isGroupingByCriteria() const
{
  return ui->radioButton_groupingCriterion->isChecked();
}

void HotellingCriterionParametersDialog::setupUi()
{
  ui->setupUi(mContent);
  getContent()->layout()->addWidget(mContent);
  ui->label_groupingCriterion->setStyleSheet("QLabel{font-size: 9pt;}");
  ui->criterionGroupingWarningLabel->setStyleSheet("QLabel{font-size: 9pt; color: red;}");

  mExcludedLayout = new FlowLayout();
  auto pageLayout = qobject_cast<QVBoxLayout*>(ui->stepTwoPage->layout());
  pageLayout->addLayout(mExcludedLayout);

  setupStep(mStep, true);

  //// Connect
  connect(ui->radioButton_groupingCriterion, &QRadioButton::toggled, [&](bool checked)
          {
            ui->comboWidget->setVisible(checked);
            if(checked)
            {
              auto ind = ui->comboBox_groupingCriterion->currentIndex();

              if(ind == -1)
                return;

              auto groups = getPcaCriterionGroups(ui->comboBox_groupingCriterion->currentData().toInt());
              onGroupCalcChanged(groups);
            }
          });
  connect(ui->radioButton_groupingCountPlot, &QRadioButton::toggled, [&](bool checked)
          {
            if(checked)
            {
              auto groups = getPcaGroups();
              onGroupCalcChanged(groups);
            }
          });
  connect(ui->radioButton_groupingManually, &QRadioButton::toggled, [&](bool checked)
          {
            if(checked)
            {
              ui->criterionGroupingWarningLabel->hide();
              ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
            }
          });
}

void HotellingCriterionParametersDialog::setupStep(int step, bool initial)
{
  if(step == 0)
  {
    Dialogs::Templates::Info::Settings s;
    s.dialogHeader = tr("Parameters of the Hotelling criterion");
    s.contentHeader = tr("Step %1 of 2. Grouping principle").arg(mStep + 1);
    s.buttonsNames = { {QDialogButtonBox::Ok, tr("Next step")},
                      {QDialogButtonBox::Cancel, tr("Cancel")} };
    applySettings(s);

    ui->radioButton_groupingCriterion->setChecked(true);
    ui->criterionGroupingWarningLabel->hide();
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
  else if(step == 1)
  {
    if(initial)//from manual hott
      ui->radioButton_groupingManually->setChecked(true);
    clearExcluded();
    //switch to page 2
    auto s = getCurrentSettings();
    s.dialogHeader = tr("Parameters of the Hotelling criterion");
    s.contentHeader = tr("Step 2 of 2. Select count of PC");
    s.buttonsNames = { {QDialogButtonBox::Ok, tr("Calculate")},
                      {QDialogButtonBox::Cancel, tr("Cancel")} };
    applySettings(s);
    QStringList excludedSamples;
    if(ui->radioButton_groupingCriterion->isChecked())
    {
      mGroups = getPcaCriterionGroups(ui->comboBox_groupingCriterion->currentData().toInt(), &excludedSamples);
    }
    else if(ui->radioButton_groupingCountPlot->isChecked())
    {
      mGroups = getPcaGroups(&excludedSamples);
    }
    else if(ui->radioButton_groupingManually->isChecked())
    {
      mGroups = getManualGroups(&excludedSamples);
    }
    setExcluded(excludedSamples);
    onGroupCalcChanged(mGroups);
    if(mGroups.size() >= 2)
    {
      Q_ASSERT(!mGroups.isEmpty());
      if(mGroups.isEmpty())
        return;
      ui->spinBox_countOfPC->setMinimum(2);
      ui->spinBox_countOfPC->setMaximum(maxPCCount());
    }
    else
    {
      ui->spinBox_countOfPC->setEnabled(false);
    }
  }
  ui->stackedWidget->setCurrentIndex(step);
}

void HotellingCriterionParametersDialog::onPCACriterionIngexChanged(int ind)
{
  if(ind == -1)
  {
    return;
  }
  auto groups = getPcaCriterionGroups(ui->comboBox_groupingCriterion->currentData().toInt());
  onGroupCalcChanged(groups);
}

void HotellingCriterionParametersDialog::onGroupCalcChanged(const QHash<QString, QList<QUuid> > &groups)
{
  if(groups.size() < 2)
  {
    ui->criterionGroupingWarningLabel->show();
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
  else
  {
    ui->criterionGroupingWarningLabel->hide();
    ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
}

QHash<QString, QList<QUuid> > HotellingCriterionParametersDialog::getPcaCriterionGroups(int criteria, QStringList *excludedSamples)
{
  if(!mPcaCriterionCache.contains(criteria))
  {
    QHash<QString, QList<QUuid> > hottGroups;
    auto samples = mModel->getEntities(TypeSample);
    for(auto& s : samples)
    {
      QString value;
      auto passport = s->getData(RolePassport).value<TPassport>();
      switch((Criteria)criteria)
      {
      case field:
        value = passport.value(PassportTags::field).toString();
        hottGroups[value] << s->getUid();
        break;
      case layer:
        value = passport.value(PassportTags::layer).toString();
        hottGroups[value] << s->getUid();
        break;
      case well:
        value = passport.value(PassportTags::well).toString();
        hottGroups[value] << s->getUid();
        break;
      default:
        if(excludedSamples)
          excludedSamples->append(sampleName(s->getUid()));
        break;
      }
    }
    QStringList excluded;
    fillExcluded(&excluded, hottGroups);
    if(excludedSamples)
      *excludedSamples = excluded;
    mPcaCriterionExcludedCache[criteria] = excluded;
    hottGroups.removeIf([](std::pair<const QString &, QList<QUuid> &> pair)->bool{return pair.second.size() < 2;});
    mPcaCriterionCache[criteria] = hottGroups;
  }
  if(excludedSamples)
    *excludedSamples = mPcaCriterionExcludedCache[criteria];
  return mPcaCriterionCache[criteria];
}

QHash<QString, QList<QUuid> > HotellingCriterionParametersDialog::getPcaGroups(QStringList* excludedSamples)
{
  if(mPcaCache)
  {
    if(excludedSamples)
      *excludedSamples = *mPcaExcludedCache;
    return *mPcaCache;
  }
  mPcaCache = new QHash<QString, QList<QUuid> >();
  mPcaExcludedCache = new QStringList();
  auto& hottGroups = *mPcaCache;
  auto samples = mModel->getEntities(TypeSample);
  auto groups = mModel->getEntities(TypeGroup);

  QSet<TEntityUid> groupsUidsSet;
  for(auto& group : groups)
    groupsUidsSet << group->getUid();

  for(auto& s : samples)
  {
    auto set = s->getData(RoleGroupsUidSet).value<TGroupsUidSet>().intersect(groupsUidsSet);
    if(set.isEmpty())
    {
      if(excludedSamples)
        excludedSamples->append(sampleName(s->getUid()));
    }
    else
    {
      for(auto& groupUid : set)
        hottGroups[mModel->getEntity(groupUid)->getData(RoleTitle).toString()] << s->getUid();
    }
  }
  QStringList excluded;
  fillExcluded(&excluded, hottGroups);
  if(excludedSamples)
    *excludedSamples = excluded;
  *mPcaExcludedCache = excluded;
  hottGroups.removeIf([](std::pair<const QString &, QList<QUuid> &> pair)->bool{return pair.second.size() < 2;});
  return hottGroups;
}

QHash<QString, QList<QUuid> > HotellingCriterionParametersDialog::getManualGroups(QStringList *excludedSamples)
{
  QHash<QString, QList<QUuid> > hottGroups;
  auto samples = mModel->getEntities(TypeSample);
  auto groups = mModel->getEntities(TypeHottelingGroup);
  groups.removeIf([](AbstractEntityDataModel::ConstDataPtr group)->bool
                  {return !group->getData(RoleGroupUserCreated).toBool();});

  QSet<TEntityUid> groupsUidsSet;
  for(auto& group : groups)
    groupsUidsSet << group->getUid();

  for(auto& s : samples)
  {
    auto set = s->getData(RoleGroupsUidSet).value<TGroupsUidSet>().intersect(groupsUidsSet);
    if(set.isEmpty())
    {
      if(excludedSamples)
        excludedSamples->append(sampleName(s->getUid()));
    }
    else
    {
      for(auto& groupUid : set)
        hottGroups[mModel->getEntity(groupUid)->getData(RoleTitle).toString()] << s->getUid();
    }
  }

  fillExcluded(excludedSamples, hottGroups);
  hottGroups.removeIf([](std::pair<const QString &, QList<QUuid> &> pair)->bool{return pair.second.size() < 2;});
  return hottGroups;
}

QString HotellingCriterionParametersDialog::sampleName(const QUuid &uid)
{
  auto ent = mModel->getEntity(uid);
  if(!ent)
    return QString();
  return ent->getData(RoleTitle).toString();
  // auto passport = ent->getData(RolePassport).value<TPassport>();
  // auto title = passport.filetitle;
  // if(title.isEmpty())
  // {
  //   title = passport.filename.section('/', -1);
  //   if(title.contains('\\'))
  //     title = title.section('\\', -1);
  // }
  // else
  // {
  //   title = passport.filetitle;
  // }
  // return title;
}

void HotellingCriterionParametersDialog::fillExcluded(QStringList *excludedSamples, const QHash<QString, QList<QUuid> > &groups)
{
  if(!excludedSamples)
    return;
  for(auto iter : groups.asKeyValueRange())
    if(iter.second.size() < 2)
      for(auto& s : iter.second)
        excludedSamples->append(sampleName(s));
}

int HotellingCriterionParametersDialog::maxPCCount()
{
  int minSum = mGroups.begin().value().size() + std::next(mGroups.begin()).value().size();
  int tmpSum = 0;
  for(auto& g1 : mGroups)
  {
    for(auto& g2 : mGroups)
    {
      if(&g1 == &g2)
        continue;
      tmpSum = g1.size() + g2.size();
      if(tmpSum < minSum)
        minSum = tmpSum;
    }
  }
  return (minSum - 2 >= 2) ? minSum - 2 : 2;
}

void HotellingCriterionParametersDialog::clearExcluded()
{
  while(mExcludedLayout->count() != 0)
  {
    auto item = mExcludedLayout->takeAt(0);
    if(auto widget = item->widget())
      widget->deleteLater();
  }
}

void HotellingCriterionParametersDialog::setExcluded(const QStringList &excluded)
{
  if(excluded.isEmpty())
  {
    ui->nonIncludedSamplesLabel->setText(tr("All samples will be included to calculation"));
    return;
  }
  else
  {
    ui->nonIncludedSamplesLabel->setText(tr("This samples will be not included to calculation:"));
  }
  for(auto& str : excluded)
  {
    auto lbl = new QLabel(str);
    mExcludedLayout->addWidget(lbl);
  }
}

int HotellingCriterionParametersDialog::step() const
{
  return mStep;
}

QHash<QString, QList<QUuid> > &HotellingCriterionParametersDialog::groups()
{
  return mGroups;
}
