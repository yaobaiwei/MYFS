#include "util/raid5.hpp"
#include <assert.h>
#include "string.h"

Raid5::Raid5(size_t block_size) {
    buf = std::string(block_size, 0);
}

void Raid5::encode(const std::string &data) {
    assert(data.length() == buf.length());
    for (size_t i = 0; i < buf.size(); i++) {
        buf[i] ^= data[i];
    }
}
