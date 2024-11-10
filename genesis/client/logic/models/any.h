#pragma once
#ifndef ANY_H
#define ANY_H

#include <typeinfo>
#include <memory>
#include <stdexcept>

class any
{
  struct base_holder
  {
    virtual ~base_holder() {}
    virtual const std::type_info& type_info() const = 0;
    virtual std::unique_ptr<base_holder> copy() const = 0;
  };

  template<typename T>
  struct holder final : base_holder
  {
    T data_;

    holder(T t) : data_(std::move(t)) {}

    const std::type_info& type_info() const override
    {
      return typeid(data_);
    }

    std::unique_ptr<base_holder> copy() const override
    {
      return std::make_unique<holder>(*this);
    }
  };
  std::unique_ptr<base_holder> held_ = nullptr;

public:
  template<typename T>
  any(T t) : held_(std::make_unique<holder<T>>(std::move(t))) {}
  any(any&& x) noexcept = default;

  template<typename U>
  U& cast() const
  {
    if (typeid(U) != held_->type_info())
      throw std::runtime_error("Bad component cast");
    return static_cast<holder<U>*>(held_.get())->data_;
  }

  template<typename U>
  bool has_value() const
  {
    return held_.operator bool();
  }

  any& operator=(any x) noexcept
  {
    held_ = std::move(x.held_);
    return *this;
  }
};

#endif
