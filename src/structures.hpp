#pragma once
#include <cstddef>
#include <cassert>

const size_t kMinChunkSize = 0x20;

// |-----------+-----------|
// | prev_size |    size |U|
// |-----------+-----------|
// |          buf          |
// |-----------------------|
// |                       |
// |-----------------------|
// |          ...          |
// |-----------------------|
struct alignas(0x10) Chunk {
  size_t prev_size;
  size_t size;
  char buf[0x10];
  
  size_t addr() { return reinterpret_cast<size_t>(this); }

  bool is_used() { return size & 1; }
  void set_used_flag() { size |= 1; }
  void clear_used_flag() { size &= ~1ull; }

  size_t get_chunk_size() { return size & ~1ull; }
  size_t get_prev_chunk_size() { return prev_size; }
  size_t get_buf_size() { return get_chunk_size() - 2 * sizeof(size_t); }

  Chunk *next() {
    size_t addr = reinterpret_cast<size_t>(this) + get_chunk_size();
    return reinterpret_cast<Chunk *>(addr);
  }
  Chunk *prev() {
    size_t addr = reinterpret_cast<size_t>(this) - get_prev_chunk_size();
    return reinterpret_cast<Chunk *>(addr);
  }

  void split(size_t first_chunk_size) {
    assert(!is_used());
    assert(first_chunk_size + kMinChunkSize <= get_chunk_size());
    Chunk *second = reinterpret_cast<Chunk *>(addr() + first_chunk_size);
    second->prev_size = first_chunk_size;
    second->size = get_chunk_size() - first_chunk_size;
    size = first_chunk_size;
  }
};

// convert request size to chunk size
inline size_t request_to_chunk_size(size_t req) {
  size_t size = req + 2 * sizeof(void *); // header size
  if (size <= kMinChunkSize) 
    return kMinChunkSize;
  if (size % 0x10 == 0)
    return size;
  return (size + 0x10) & (~0xfull);
}

inline Chunk *buf_to_chunk_ptr(void *ptr) {
  size_t addr = reinterpret_cast<size_t>(ptr);
  return reinterpret_cast<Chunk *>(addr - 2 * sizeof(void *));
}


// 1つのチャンクを2つに切り分け、先頭のチャンクを使用中にする。
// 1つ目のチャンクの大きさが size で、
// 2つ目のチャンクの大きさが get_chunk_size(chunk) - size となる。
// inline void *split_and_allocate_first(Chunk *chunk, size_t size) {
//   assert(!is_used(chunk));
//   assert(kMinChunkSize + size <= get_chunk_size(chunk));
//   size_t addr = reinterpret_cast<size_t>(chunk);
//   Chunk *second = reinterpret_cast<Chunk *>(addr + size);
//   second->prev_size = size;
//   second->size = get_chunk_size(chunk) - size;
//   chunk->size = size;
//   set_used_flag(chunk);
// }