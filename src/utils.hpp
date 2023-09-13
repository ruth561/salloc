#pragma once
#include <cstddef>

#include "structures.hpp"

// align is a power of 2
void *next_aligned_address(void *ptr, size_t align);