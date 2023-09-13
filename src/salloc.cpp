
#include "heaphook/heaphook.hpp"
#include "heaphook/utils.hpp"

#include "utils.hpp"
#include "structures.hpp"

using namespace heaphook;

size_t kMemPoolBaseAddress;

void write_chunk_to_stderr(Chunk *chunk) {
  size_t addr = reinterpret_cast<size_t>(chunk);
  size_t offset = addr - kMemPoolBaseAddress;
  write_to_stderr("Chunk(", reinterpret_cast<void *>(offset), "): prev_size = ", 
    reinterpret_cast<void *>(chunk->prev_size), 
    ", size = ", reinterpret_cast<void *>(chunk->size), ", ", 
    chunk->is_used() ? "(USED)" : "(NOT USED)", "\n");
}

class SimpleAllocator : public GlobalAllocator {
  Chunk *top_;
  Chunk *first_chunk_;
  alignas(0x1000) char mem_pool_[kMemPoolSize];

  void *do_alloc(size_t bytes, size_t align) override {
    size_t size = request_to_chunk_size(bytes); // size bytes chunk has a buf larger than bytes
    if (size > kMaxChunkSize) {
      return nullptr;
    }
    if (align == 1) { // no alignment
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
    } else {
      Chunk *tmp_chunk = buf_to_chunk_ptr(do_alloc(size + kChunkHdrSize + kMinChunkSize + align, 1));
      size_t buf_addr = reinterpret_cast<size_t>(tmp_chunk->buf);
      if (buf_addr % align == 0) {
        return tmp_chunk->buf;
      } else {
        tmp_chunk->clear_used_flag();
        size_t return_addr = (buf_addr + kMinChunkSize + kChunkHdrSize + align) & ~(align - 1);
        tmp_chunk->split(return_addr - buf_addr - kChunkHdrSize + 0x10);
        auto return_chunk = tmp_chunk->next();
        return_chunk->set_used_flag();
        return return_chunk->buf;
      }
    }
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
    first_chunk_ = reinterpret_cast<Chunk *>(mem_pool_);
    first_chunk_->prev_size = 0;
    first_chunk_->size = kMemPoolSize;
    first_chunk_->split(kMinChunkSize);
    first_chunk_->set_used_flag();
    top_ = first_chunk_->next();
    strcpy(first_chunk_->buf, "FIRST CHUNK");
    strcpy(top_->buf, "TOP!!");
  }
};

GlobalAllocator &GlobalAllocator::get_instance() {
  static SimpleAllocator allocator;
  return allocator;
}
