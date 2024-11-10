#pragma once
#ifndef LIBRARY_STRUCTURES_H
#define LIBRARY_STRUCTURES_H

#include "logic/enums.h"

namespace Structures
{
  struct LibraryFilterState
  {
    QString Name;
    QString Group;
    LibraryFilter::FileType Type = LibraryFilter::FileType::All;
    LibraryFilter::Classifiers Classifier = LibraryFilter::Classifiers::All;
    LibraryFilter::Specifics Specific = LibraryFilter::Specifics::All;
  };

  struct KeyString
  {
    QString shortName;
    QString fullName;
    int id;
  };
}
#endif
