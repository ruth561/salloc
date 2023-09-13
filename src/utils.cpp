#include "utils.hpp"

void *next_aligned_address(void *ptr, size_t align) {
  size_t addr = reinterpret_cast<size_t>(ptr);
  if (addr % align != 0) {
    addr = (addr + align) & ~(align - 1);
  }
  return reinterpret_cast<void *>(addr);
}