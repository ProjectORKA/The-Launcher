#pragma once

#include "Basics.hpp"
#include "minizip-ng/mz.h"
#include "minizip-ng/mz_compat.h"

void extractZip(const Path& archive, const Path& target = ".");