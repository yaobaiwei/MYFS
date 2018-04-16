#pragma once

#include <unistd.h>
#include <string>

struct Raid5 {
    Raid5(size_t block_size);

    void encode(const std::string &);

    std::string buf;
};
