#pragma once
#ifndef SERVICE_H
#define SERVICE_H

namespace Service
{
  class IService
  {
  public:
    IService() = default;
    virtual ~IService() = default;

    IService(const IService& copyFrom) = default;
    IService& operator=(const IService& copyFrom) = default;
    IService(IService&&) noexcept = default;
    IService& operator=(IService&&) = default;
  };
}

#endif