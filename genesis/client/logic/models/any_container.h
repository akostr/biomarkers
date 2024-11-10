#pragma once
#ifndef ANY_CONTAINER_H
#define ANY_CONTAINER_H

#include "any.h"

#include <optional>
#include <utility>
#include <tuple>
#include <unordered_map>
#include <stdexcept>

namespace Model
{
  class AnyContainer
  {
  public:
    template<typename Component>
    void Set(Component comp)
    {
      components_.insert_or_assign(typeid(Component).hash_code(), std::forward<Component>(comp));
    }

    template<typename Component>
    std::optional<std::reference_wrapper<Component>> Get()
    {
      const auto hash = typeid(Component).hash_code();
      if (const auto& iter = components_.find(hash); iter != components_.end())
      {
        auto& [h, item] = *iter;
        if (item.has_value<Component>())
          return item.cast<Component>();
        throw std::runtime_error("Bad component cast!");
      }
      return std::nullopt;
    }

    template<typename Component>
    std::optional<Component> Get() const
    {
      const auto hash = typeid(Component).hash_code();
      if (const auto& iter = components_.find(hash); iter != components_.end())
      {
        auto& [h, item] = *iter;
        if (item.has_value<Component>())
          return item.cast<Component>();
        return std::nullopt;
      }
    }

    template<typename... Components>
    bool Contains()
    {
      const auto tuple = std::make_tuple(Get<Components>()...);
      bool result = true;
      std::apply([&result](auto&&... args)
        { ((result &= args.has_value()), ...); }, tuple);
      return result;
    }

    template<typename... Components, typename Pred>
    void Exec(Pred func)
    {
      // Warning it may crash if component not in container.
      //
      auto tuple = std::make_tuple(Get<Components>().value()...);
      std::apply(func, tuple);
    }

  private:
    std::unordered_map<size_t, any> components_;
  };
}

#endif
