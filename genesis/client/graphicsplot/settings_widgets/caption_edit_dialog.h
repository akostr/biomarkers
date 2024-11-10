#ifndef DISABLE_STANDARD_VIEWS

#pragma once

#include <standard_dialogs/name_edit_dialog.h>

class GPLayerable;
class GPAbstractPlottable;
class GPAxis;
class QTabBar;
class QCheckBox;
class QComboBox;

class CaptionEditDialog : public NameEditDialog
{
  Q_OBJECT

public:
  CaptionEditDialog(QWidget* parent, const QString& caption, const QString& hint, const QString& defaultValue = QString::null);
  ~CaptionEditDialog();

  virtual void UpdateUi();

  void SetHasLine(bool has);
  bool GetHasLine();

  void SetCurvesList(const QList<GPAbstractPlottable*>& targetsList);
  void SetAxesList(const QList<GPAxis*>& targetsList);

  void SetTarget(GPLayerable* target);
  GPLayerable* GetTarget();

protected:
  QCheckBox* cbHasLine;
  QTabBar*   tbTargetType;
  QComboBox* cbTargetCurve;
  QComboBox* cbTargetAxis;
};

#endif
