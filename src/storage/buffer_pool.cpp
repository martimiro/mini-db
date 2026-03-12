#include "storage/buffer_pool.h"
#include <stdexcept>

// Constructor
BufferPool::BufferPool(DiskManager& diskManager, uint32_t poolSize)
    : diskManager_(diskManager), poolSize_(poolSize)
    {
    frames_.resize(poolSize);
    for (uint32_t i = 0; i < poolSize; i++) {
        lruList_.push_back(i);
        lruMap_[i] = std::prev(lruList_.end());
    }
}
