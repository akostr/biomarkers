#include "service_locator.h"

namespace Service
{
    ServiceLocator::~ServiceLocator()
    {
      for (auto& [name, service] : services)
        delete service;

      services.clear();
      serviceNames.clear();
    }
}