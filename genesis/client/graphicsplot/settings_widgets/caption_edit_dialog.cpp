#include "caption_edit_dialog.h"

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
#include <licensing/licensing.h>
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

#ifndef DISABLE_STANDARD_VIEWS

#include <graphicsplot/graphicsplot.h>
#include <standard_style/style.h>

#include <QTabBar>

CaptionEditDialog::CaptionEditDialog(QWidget *parent, const QString &caption, const QString &hint, const QString &defaultValue)
  : NameEditDialog(parent,
                   caption,
                   hint,
                   defaultValue)
{
  QVBoxLayout* vl = new QVBoxLayout();
  vl->setContentsMargins(9, 12, 9, 9);
  vl->setSpacing(10);

  QHBoxLayout* hl = new QHBoxLayout();
  hl->setSpacing(10);
  vl->addLayout(hl);

  {
    QLabel* label = new QLabel(tr("Attached object:"), this);
    hl->addWidget(label, 0);

    tbTargetType = new QTabBar(this);
    tbTargetType->addTab(tr("Curve"));
    tbTargetType->addTab(tr("Axis"));
    tbTargetType->setStyleSheet(Style::GetTabbarVividStyle());
    hl->addWidget(tbTargetType, 0);
  }

  {
    cbTargetCurve = new QComboBox(this);
    vl->addWidget(cbTargetCurve);

    cbTargetAxis = new QComboBox(this);
    cbTargetAxis->setVisible(false);
    vl->addWidget(cbTargetAxis);
  }

  Layout->insertLayout(Layout->count() - 1, vl);

  cbHasLine = new QCheckBox(tr("Line"), this);
  cbHasLine->setStyleSheet("margin: 9 9 0 9;");
  Layout->insertWidget(Layout->count() - 1, cbHasLine);

#ifdef CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API
  CET_LICENSE_CHECK_OR_INVALIDATE_PTR(cbHasLine);
#endif // ~CODEBASE_FEATURE_LICENSE_EMBEDDMENT_API

  connect(cbTargetCurve, &QComboBox::currentTextChanged, this, &CaptionEditDialog::UpdateUi);
  connect(cbTargetAxis,  &QComboBox::currentTextChanged, this, &CaptionEditDialog::UpdateUi);
  connect(tbTargetType,  &QTabBar::currentChanged, this, [this](int index){
    cbTargetCurve->setVisible(index == 0);
    cbTargetAxis->setVisible(index == 1);
    UpdateUi();
  });
}

CaptionEditDialog::~CaptionEditDialog()
{

}

void CaptionEditDialog::UpdateUi()
{
  NameEditDialog::UpdateUi();
  if (Buttons->button(QDialogButtonBox::Ok)->isEnabled())
  {
    Buttons->button(QDialogButtonBox::Ok)->setEnabled(GetTarget() != nullptr);
  }
}

void CaptionEditDialog::SetHasLine(bool has)
{
  cbHasLine->setChecked(has);
}

bool CaptionEditDialog::GetHasLine()
{
  return cbHasLine->isChecked();
}

void CaptionEditDialog::SetCurvesList(const QList<GPAbstractPlottable*>& targetsList)
{
  cbTargetCurve->clear();
  cbTargetCurve->addItem("", (qlonglong)nullptr);

  for (const auto& target : targetsList)
  {
    if (auto curve = qobject_cast<GPCurve*>(target))
    {
      QString name = curve->name();
      if (!name.isEmpty())
        cbTargetCurve->addItem(name, (qlonglong)curve);
    }
  }
}

void CaptionEditDialog::SetAxesList(const QList<GPAxis*>& targetsList)
{
  cbTargetAxis->clear();
  cbTargetAxis->addItem("", (qlonglong)nullptr);

  for (const auto& axis : targetsList)
  {
    QString name = axis->label();
    if (!name.isEmpty())
      cbTargetAxis->addItem(name, (qlonglong)axis);
  }
}

void CaptionEditDialog::SetTarget(GPLayerable* target)
{
  if (auto curve = qobject_cast<GPCurve*>(target))
    cbTargetCurve->setCurrentText(curve->name());
  else if (auto axis = qobject_cast<GPAxis*>(target))
    cbTargetAxis->setCurrentText(axis->label());
}

GPLayerable* CaptionEditDialog::GetTarget()
{
  if (tbTargetType->currentIndex() == 0)
    return (GPLayerable*)cbTargetCurve->currentData().toLongLong();
  if (tbTargetType->currentIndex() == 1)
    return (GPLayerable*)cbTargetAxis->currentData().toLongLong();
  return nullptr;
}

#endif
