#pragma once
#include <cstddef>

struct Range {
    size_t start;
    size_t end;

    struct Iterator {
        size_t current;
        explicit Iterator(size_t val) : current(val) {}
        size_t operator*() const { return current; }
        Iterator& operator++() { ++current; return *this; }
        bool operator!=(const Iterator& other) const { return current < other.current; }
    };

    Iterator begin() const { return Iterator(start); }
    Iterator end() const { return Iterator(end); }
};