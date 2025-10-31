#include "logger.hpp"

#ifndef NDEBUG
#define LEVEL debug
#else
#define LEVEL warn
#endif

namespace engine {
  DEFINE_LOGGER("engine", LEVEL)
}