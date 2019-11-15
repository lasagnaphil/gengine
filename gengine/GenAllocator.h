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

#ifdef USE_SHARED_PTR
#include <memory>
template <class T>
using Ref = std::shared_ptr<T>;

struct Resources {
    template <class T, class ...Args>
    static Ref<T> make(Args&&... args) {
        return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
    }
};

#else

#if __cplusplus == 201402L
#define IF_CONSTEXPR if
#else
#define IF_CONSTEXPR if constexpr
#endif

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

    bool operator==(const Ref<T>& other) const {
        return index == other.index && generation == other.generation;
    }
    bool operator!=(const Ref<T>& other) const {
        return !((*this) == other);
    }

    T* operator->() const;
    T& operator*() const;

    T* get();
    T* tryGet();
    void release();
    void reset() {
        release();
    }
};

template <typename T>
struct GenAllocator {

    struct ItemNode {
        std::aligned_storage_t<sizeof(T), alignof(T)> bytes;
        uint32_t nextIndex;
        uint32_t generation;
    };

    std::vector<ItemNode> nodes;

    uint32_t count;
    uint32_t capacity;

    uint32_t firstAvailable;

    GenAllocator(uint32_t capacity = 0) :
        count(0), capacity(capacity), nodes(capacity), firstAvailable(0)
    {
        for (uint32_t i = 0; i < capacity; ++i) {
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 0;
        }
    }

    void dispose() {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (nodes[i].generation == 0) {
                // This doesn't work in C++14 because of the lack of constexpr if
                // So we need to manually free resources before deleting allocator
                IF_CONSTEXPR(std::is_base_of<IDisposable, T>()) {
                    reinterpret_cast<T*>(&nodes[i].bytes)->dispose();
                }
            }
        }
    }

    void expand(uint32_t newCapacity) {
        assert (newCapacity >= capacity);

        nodes.resize(newCapacity);

        for (uint32_t i = capacity; i < newCapacity; ++i) {
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 0;
        }

        capacity = newCapacity;
    }

    template <class ...Args>
    Ref<T> make(Args&&... args) {
        // if the item list is full
        if (firstAvailable == capacity) {
            expand(capacity == 0? 4 : capacity * 2);
        }

        // delete node from free list
        uint32_t newIndex = firstAvailable;
        ItemNode& node = nodes[newIndex];
        new (&node.bytes) T(std::forward<Args>(args)...);
        firstAvailable = node.nextIndex;

        node.generation++;

        count++;

        // also return the reference object of the resource
        return Ref<T> {newIndex, node.generation};
    }

    bool has(Ref<T> ref) const {
        ItemNode& node = nodes[ref.index];
        return node.generation != 0 && node.generation == ref.generation;
    }

    const T* get(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        assert(node.generation != 0);
        assert(node.generation == ref.generation);

        return reinterpret_cast<T*>(&node.bytes);
    }

    T* get(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(node.generation != 0);
        assert(node.generation == ref.generation);

        return reinterpret_cast<T*>(&node.bytes);
    }

    const T* tryGet(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        if (node.generation != 0 && node.generation == ref.generation) {
            return reinterpret_cast<T*>(&node.bytes);
        }
        else {
            return nullptr;
        }
    }

    T* tryGet(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        if (node.generation != 0 && node.generation == ref.generation) {
            return reinterpret_cast<T*>(&node.bytes);
        }
        else {
            return nullptr;
        }
    }

    void release(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(node.generation != 0);
        assert(node.generation == ref.generation);

        std::swap(node.nextIndex, firstAvailable);
        IF_CONSTEXPR(std::is_base_of<IDisposable, T>()) {
            reinterpret_cast<T*>(&node.bytes)->dispose();
        }

        count--;
    }

    template <class Fun>
    void forEach(Fun&& fun) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (nodes[i].generation != 0) {
                T* data = reinterpret_cast<T*>(&nodes[i].bytes);
                Ref<T> ref = {i, nodes[i].generation};
                fun(*data, ref);
            }
        }
    }

    template <class Fun>
    void forEachUntil(Fun&& fun) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (nodes[i].generation != 0) {
                T* data = reinterpret_cast<T*>(&nodes[i].bytes);
                Ref<T> ref = {i, nodes[i].generation};
                bool end = fun(*data, ref);
                if (end) return;
            }
        }
    }
};

struct Resources {
    friend class constructor;
    struct constructor {
        constructor();
    };
    static constructor cons;

    template <class T>
    static GenAllocator<T>& getStorage();

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
#endif

#endif //MOTION_EDITING_GLOBALSTORAGE_H
