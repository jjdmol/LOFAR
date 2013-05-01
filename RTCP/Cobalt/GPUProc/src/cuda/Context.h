#pragma once

#include <boost/shared_ptr.hpp>

class Device;

class Context
{
public:
  Context(const Device& device, unsigned flags = 0);
  void setCurrent() const;
private:
  struct Impl;
  boost::shared_ptr<Impl> _impl;
};


