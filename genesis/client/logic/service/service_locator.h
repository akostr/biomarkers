#pragma once
#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

#include <functional>
#include <unordered_map>
#include <string>
#include <mutex>

#include "iservice.h"

namespace Service
{
  class ServiceLocator
  {
    using IServiceFunctor = std::function<IService* ()>;
    using ServiceClassNames = std::unordered_map<std::string, IServiceFunctor>;
    using ServiceInstances = std::unordered_map<std::string, IService*>;
    using Ptr = std::unique_ptr<ServiceLocator>;

  public:
    ServiceLocator() = default;
    ~ServiceLocator();

    static ServiceLocator& Instance()
    {
      if (!instance_)
      {
        std::once_flag flag;
        std::call_once(flag, [&]()
          {
            instance_ = std::make_unique<ServiceLocator>();
          });
      }

      return *instance_;
    }

    static void DestroyInstance()
    {
      instance_.reset();
    }

    ServiceLocator(const ServiceLocator&) = delete;
    ServiceLocator(ServiceLocator&&) = delete;

    ServiceLocator& operator=(const ServiceLocator&) = delete;
    ServiceLocator& operator=(ServiceLocator&&) = delete;

  public:
    template<typename TServiceInterface, typename TServiceImplementation>
    void RegisterService()
    {
      IServiceFunctor newService = newFunction<TServiceImplementation>();
      RegisterServiceImplementation<TServiceInterface>(newService);
    }

    template<typename TServiceTarget, typename TServiceInterface, typename TServiceImplementation>
    void RegisterService()
    {
      IServiceFunctor newService = newFunction<TServiceImplementation>();
      RegisterServiceTarget<TServiceTarget, TServiceInterface>(newService);
    }

    template<typename TServiceInterfaceOrImplementation>
    TServiceInterfaceOrImplementation* Resolve()
    {
      return resolve_impl<TServiceInterfaceOrImplementation>(typeid(TServiceInterfaceOrImplementation).name());
    }

    template<typename TServiceTarget, typename TServiceInterface>
    TServiceInterface* Resolve()
    {
      return resolve_impl<TServiceInterface>(
        std::string(typeid(TServiceInterface).name())
        + std::string(typeid(TServiceTarget).name())
        );
    }

  private:
    template<typename TServiceImplementation>
    void RegisterImplementation()
    {
      std::string id = typeid(TServiceImplementation).name();
      IServiceFunctor newService = newFunction<TServiceImplementation>();
      serviceNames.emplace(id, newService);
    }

    template<typename TServiceInterface>
    void RegisterServiceImplementation(const IServiceFunctor& newService)
    {
      serviceNames.emplace(typeid(TServiceInterface).name(), newService);
    }

    template<typename TServiceTarget, typename TServiceInterface>
    void RegisterServiceTarget(const IServiceFunctor& newService)
    {
      serviceNames.emplace(
        std::string(typeid(TServiceInterface).name())
        + std::string(typeid(TServiceTarget).name())
        , newService
      );
    }

  private:
    static inline Ptr instance_ = nullptr;
    ServiceInstances services;
    ServiceClassNames serviceNames;

    template<class TServiceInterface>
    TServiceInterface* resolve_impl(const std::string& name)
    {
      if (const auto foundService = services.find(name); foundService != services.end())
      {
        const auto& [name, service] = *foundService;
        return dynamic_cast<TServiceInterface*>(service);
      }

      const auto foundClass = serviceNames.find(name);
      if (foundClass == serviceNames.end())
      {
        return nullptr;
      }

      TServiceInterface* service(dynamic_cast<TServiceInterface*>(foundClass->second()));
      services.try_emplace(name, service);
      return service;
    }

    template<typename T>
    std::function<T* ()> newFunction()
    {
      return []() {return new T(); };
    }
  };
}
#endif
