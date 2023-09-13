
#include "heaphook/heaphook.hpp"
#include "heaphook/utils.hpp"

#include "utils.hpp"
#include "structures.hpp"

using namespace heaphook;

constexpr size_t kMemPoolSize = 0x4000000ull;
size_t kMemPoolBaseAddress;

void write_chunk_to_stderr(Chunk *chunk) {
  size_t addr = reinterpret_cast<size_t>(chunk);
  size_t offset = addr - kMemPoolBaseAddress;
  write_to_stderr("Chunk(", reinterpret_cast<void *>(offset), "): prev_size = ", 
    reinterpret_cast<void *>(chunk->prev_size), 
    ", size = ", reinterpret_cast<void *>(chunk->size), ", ", 
    chunk->is_used() ? "(USED) > " : "(NOT USED) > ", " ", chunk->buf, "\n");
}

class SimpleAllocator : public GlobalAllocator {
  Chunk *top_;
  Chunk *first_chunk_;
  alignas(0x1000) char mem_pool_[kMemPoolSize];

  void *do_alloc(size_t bytes, size_t align) override {
    if (align > 1) {
      write_to_stderr("cannot align memory!\n");
      return nullptr;
    }
    size_t size = request_to_chunk_size(bytes); // size bytes chunk has a buf larger than bytes
    Chunk *chunk = first_chunk_;
    while (true) {
      if (!chunk->is_used() && size <= chunk->get_chunk_size()) {
        break;
      }
      chunk = chunk->next();
    }
    if (size + kMinChunkSize <= chunk->get_chunk_size()) {
      chunk->split(size);
    }
    chunk->set_used_flag();
    void *retval = reinterpret_cast<void *>(chunk->buf);
    return retval;
  }
 
  void do_dealloc(void *ptr) override {
    Chunk *chunk = buf_to_chunk_ptr(ptr);
    assert(chunk->is_used());
    chunk->clear_used_flag();
  }

  size_t do_get_block_size(void *ptr) override {
    Chunk *chunk = buf_to_chunk_ptr(ptr);
    return chunk->get_buf_size();
  }

public:
  SimpleAllocator() : mem_pool_{} {
    kMemPoolBaseAddress = reinterpret_cast<size_t>(mem_pool_);
    memset(mem_pool_, 0, kMemPoolSize);
    write_to_stderr("\n🐬\n");
    first_chunk_ = reinterpret_cast<Chunk *>(mem_pool_);
    first_chunk_->prev_size = 0;
    first_chunk_->size = kMemPoolSize;
    first_chunk_->split(kMinChunkSize);
    first_chunk_->set_used_flag();
    top_ = first_chunk_->next();
    strcpy(first_chunk_->buf, "FIRST CHUNK");
    strcpy(top_->buf, "TOP!!");

    write_chunk_to_stderr(first_chunk_);
    write_chunk_to_stderr(top_);
  }
};

GlobalAllocator &GlobalAllocator::get_instance() {
  static SimpleAllocator allocator;
  return allocator;
}
