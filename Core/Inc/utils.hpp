#pragma once
#include <array>
#include <cstdint>

template<typename T>
void toType(const std::array<uint8_t, 8>& data, T& var) {
    uint8_t* ptr = (uint8_t*)&var;
    for (size_t i = 0; i < sizeof(T); ++i, ++ptr) {
        *ptr = data[i];
    }
}

template<typename T>
void toArray(std::array<uint8_t, 8>& data, const T& var) {
    uint8_t* ptr = (uint8_t*)&var;
    for (size_t i = 0; i < sizeof(T); ++i, ++ptr) {
        data[i] = *ptr;
    }
}
