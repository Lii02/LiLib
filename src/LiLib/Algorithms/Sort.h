#pragma once
#include "LiLib/Containers/Array.h"

namespace LiLib {

/*
 * @brief Stable sort (insertion sort for short runs, merge sort above). Elements that compare
 *        equal keep their original order, which matters for draw ordering.
 */
template<typename T, typename TLess>
void StableSort(Array<T>& values, TLess less) {
    const size_t count = values.Size();
    if(count < 2) {
        return;
    }

    constexpr size_t RunLength = 16;
    T* data = values.Data();

    for(size_t start = 0; start < count; start += RunLength) {
        const size_t stop = Min(start + RunLength, count);
        for(size_t i = start + 1; i < stop; ++i) {
            T value = std::move(data[i]);
            size_t j = i;
            while(j > start && less(value, data[j - 1])) {
                data[j] = std::move(data[j - 1]);
                --j;
            }
            data[j] = std::move(value);
        }
    }

    if(count <= RunLength) {
        return;
    }

    Array<T> buffer;
    buffer.Reserve(count);
    for(size_t i = 0; i < count; ++i) {
        buffer.Push(data[i]);
    }

    T* source = data;
    T* target = buffer.Data();
    bool resultInBuffer = false;
    for(size_t width = RunLength; width < count; width *= 2) {
        for(size_t left = 0; left < count; left += width * 2) {
            const size_t middle = Min(left + width, count);
            const size_t right = Min(left + width * 2, count);
            size_t a = left;
            size_t b = middle;
            size_t out = left;
            while(a < middle && b < right) {
                if(less(source[b], source[a])) {
                    target[out++] = std::move(source[b++]);
                } else {
                    target[out++] = std::move(source[a++]);
                }
            }
            while(a < middle) {
                target[out++] = std::move(source[a++]);
            }
            while(b < right) {
                target[out++] = std::move(source[b++]);
            }
        }
        Swap(source, target);
        resultInBuffer = !resultInBuffer;
    }

    if(resultInBuffer) {
        for(size_t i = 0; i < count; ++i) {
            data[i] = std::move(buffer[i]);
        }
    }
}

template<typename T>
void StableSort(Array<T>& values) {
    StableSort(values, [](const T& a, const T& b) { return a < b; });
}

} // namespace LiLib
