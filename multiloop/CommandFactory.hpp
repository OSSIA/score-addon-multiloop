#pragma once
#include <score/command/Command.hpp>

namespace multiloop
{
inline const CommandGroupKey& CommandFactoryName()
{
  static const CommandGroupKey key{"multiloop"};
  return key;
}
}
