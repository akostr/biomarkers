#pragma once
#ifndef VIEW_PAGE_PROJECT_MARKUP_VERSION_PAGE_H
#define VIEW_PAGE_PROJECT_MARKUP_VERSION_PAGE_H

#include "view.h"

namespace Views
{
  class ViewPageProjectMarkupVersionPage final : public View
  {
    Q_OBJECT

  public:
    explicit ViewPageProjectMarkupVersionPage(QWidget* parent = nullptr);
    ~ViewPageProjectMarkupVersionPage() override = default;

  private:
    void SetupUi();
  };
}
#endif