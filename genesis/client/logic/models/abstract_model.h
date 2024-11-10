#pragma once
#ifndef ABSTRACT_MODEL_H
#define ABSTRACT_MODEL_H

#include <unordered_map>
#include <string>
#include <cassert>
#include <list>

#include "any_container.h"

namespace Model
{
  class AbstractModel
  {
  public:
    AbstractModel() = default;
    virtual ~AbstractModel() = default;

    template<typename Component>
    void Set(const std::string& name, Component comp)
    {
      data_[name].Set(std::forward<Component>(comp));
    }

    template<typename Component>
    Component& Get(const std::string& name)
    {
      if (auto it = data_.find(name); it!= data_.end())
      {
        auto& [name, container] = *it;
        auto comp = container.Get<Component>();
        // "Wrong component!"
        assert(comp.has_value());
        return comp.value();
      }
      // "Name not found in container!"
      assert(false);
    }

    template<typename Component>
    Component Get(const std::string& name) const
    {
      if (auto it = data_.find(name); it != data_.end())
      {
        auto& [name, container] = *it;
        auto comp = container.Get<Component>();
        // "Wrong component!"
        assert(comp.has_value());
        return comp.value();
      }
      // "Name not found in container!"
      assert(false);
    }

    template<typename... Components, typename Pred>
    void ForEach(Pred func)
    {
      for (auto& [name, container] : data_)
      {
        if (container.Contains<Components...>())
        {
          container.Exec<Components...>(func);
        }
      }
    }

    template<typename... Components, typename Pred>
    void Exec(std::string& name, Pred func)
    {
      if (const auto& it = data_.find(name); it != data_.end())
      {
        auto& [h, container] = *it;
        if (container.Contains<Components...>())
        {
          container.Exec<Components...>(func);
        }
      }
    }

    void Clean()
    {
      data_.clear();
    }

    template<typename... Components, typename Pred>
    void EraseEach(Pred func)
    {
      std::list<std::string> erase_list;
      for (auto& [name, container] : data_)
      {
        if (container.Contains<Components...>())
        {
          auto tuple = std::make_tuple(container.Get<Components>().value()...);
          if (std::apply(func, tuple))
          {
            erase_list.emplace_back(name);
          }
        }
      }
      for (auto name : erase_list)
        data_.erase(name);
    }

    void Erase(const std::string& name)
    {
      data_.erase(name);
    }

  private:
    std::unordered_map<std::string, AnyContainer> data_;
  };
}

#endif
