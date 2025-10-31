#include "logger.hpp"

#ifndef NDEBUG
#define LEVEL debug
#else
#define LEVEL warn
#endif

namespace gl {
  DEFINE_LOGGER("gl", LEVEL)
}