//
// Created by lasagnaphil on 2/21/18.
//

#ifndef MOTION_EDITING_GLOBALSTORAGE_H
#define MOTION_EDITING_GLOBALSTORAGE_H

#include <cstring>
#include <cassert>
#include <utility>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <optional>

#include "IDisposable.h"

class TypeRegistry {
public:
    template <typename T> static uint16_t getID();
};

template <typename T>
struct Ref {
    uint32_t index;
    uint32_t generation;

    static Ref fromInt64(int64_t data) {
        return *reinterpret_cast<Ref*>(&data);
    }
    int64_t toInt64() {
        return *reinterpret_cast<int64_t*>(this);
    }

    static Ref null() {
        return Ref {};
    }

    bool isNull() const {
        return generation == 0;
    }

    explicit operator bool() const {
        return generation != 0;
    }

    T* operator->() const;
    T& operator*() const;

    T* get();
    T* tryGet();
    void release();
};

template <typename T>
struct Storage {

    struct ItemNode {
        std::optional<T> item;
        uint32_t nextIndex;
        uint32_t generation;
    };

    std::vector<ItemNode> nodes;

    uint32_t count;
    uint32_t capacity;

    uint32_t firstAvailable;

    Storage(uint32_t capacity = 1) :
        count(0), capacity(capacity), nodes(capacity), firstAvailable(0)
    {
        for (uint32_t i = 0; i < capacity; ++i) {
            nodes[i].item = std::nullopt;
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 1;
        }
    }

    ~Storage() {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (nodes[i].item) {
                if constexpr (std::is_base_of<IDisposable, T>()) {
                    nodes[i].item->dispose();
                }
            }
        }
    }

    void expand(uint32_t newCapacity) {
        assert (newCapacity >= capacity);

        nodes.resize(newCapacity);

        for (uint32_t i = capacity; i < newCapacity; ++i) {
            nodes[i].item = std::nullopt;
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 1;
        }

        capacity = newCapacity;
    }

    template <class ...Args>
    Ref<T> make(Args... args) {
        // if the item list is full
        if (firstAvailable == capacity) {
            expand(capacity * 2);
        }

        // delete node from free list
        uint32_t newIndex = firstAvailable;
        ItemNode& node = nodes[newIndex];
        node.item = T(args...);
        firstAvailable = node.nextIndex;

        node.generation++;

        count++;

        Ref<T> ref;

        // also return the reference object of the resource
        return Ref<T> {newIndex, node.generation};
    }

    bool has(Ref<T> ref) const {
        ItemNode& node = nodes[ref.index];
        return !node.free && node.generation == ref.generation;
    }

    const T* get(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        assert(node.item);
        assert(node.generation == ref.generation);

        return &node.item;
    }

    T* get(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(node.item);
        assert(node.generation == ref.generation);

        return &*node.item;
    }

    const T* tryGet(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        if (node.item && node.generation == ref.generation) {
            return &*node.item;
        }
        else {
            return nullptr;
        }
    }

    T* tryGet(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        if (node.item && node.generation == ref.generation) {
            return &*node.item;
        }
        else {
            return nullptr;
        }
    }

    void release(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(node.item);
        assert(node.generation == ref.generation);

        std::swap(node.nextIndex, firstAvailable);
        if constexpr (std::is_base_of<IDisposable, T>()) {
            node.item->dispose();
        }
        node.item = std::nullopt;

        count--;
    }
};

struct Resources {
    template <class T>
    static Storage<T>& getStorage();

    template <class T>
    static T* get(Ref<T> ref) {
        return getStorage<T>().get(ref);
    }

    template <class T>
    static T* tryGet(Ref<T> ref) {
        return getStorage<T>().tryGet(ref);
    }

    template <class T, class ...Args>
    static Ref<T> make(Args... args) {
        return getStorage<T>().make(args...);
    }

    template <class T>
    static void release(Ref<T> ref) {
        return getStorage<T>().release(ref);
    }

};
#endif //MOTION_EDITING_GLOBALSTORAGE_H
