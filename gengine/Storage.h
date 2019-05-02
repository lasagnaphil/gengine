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
#include "IDisposable.h"

class TypeRegistry {
public:
    template <typename T> static uint16_t getID();
};

template <typename T>
struct Ref {
    uint32_t index;
    uint16_t generation;
    uint16_t tid;

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
        return tid == 0;
    }

    explicit operator bool() const {
        return tid != 0;
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
        T item;
        uint32_t nextIndex;
        uint16_t generation;
        uint16_t free : 1;
    };

    ItemNode* nodes;

    uint32_t count;
    uint32_t capacity;

    uint32_t firstAvailable;

    uint16_t tid;

    Storage(uint32_t capacity = 1) :
        count(0), capacity(capacity), firstAvailable(0), tid(TypeRegistry::getID<T>())
    {
        nodes = (ItemNode*) malloc(sizeof(ItemNode) * capacity);
        for (uint32_t i = 0; i < capacity; ++i) {
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 0;
            nodes[i].free = 1;
        }
    }

    ~Storage() {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (!nodes[i].free) {
                if constexpr (std::is_base_of<IDisposable, T>()) {
                    nodes[i].item.dispose();
                }
                nodes[i].item.~T();
            }
        }
        free(nodes);
    }

    void expand(uint32_t newCapacity) {
        assert (newCapacity >= capacity);

        ItemNode* tempNodes = nodes;
        nodes = (ItemNode*) malloc(sizeof(ItemNode) * newCapacity);
        memcpy(nodes, tempNodes, sizeof(ItemNode) * capacity);

        for (uint32_t i = capacity; i < newCapacity; ++i) {
            nodes[i].nextIndex = i + 1;
            nodes[i].generation = 0;
            nodes[i].free = 1;
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
        new (&node.item) T(args...);
        firstAvailable = node.nextIndex;

        node.generation++;
        node.free = 0;

        count++;

        Ref<T> ref;

        // also return the reference object of the resource
        ref.index = newIndex;
        ref.generation = node.generation;
        ref.tid = tid;

        return ref;
    }

    bool has(Ref<T> ref) const {
        ItemNode& node = nodes[ref.index];
        return !node.free && node.generation == ref.generation;
    }

    const T* get(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        assert(!node.free);
        assert(node.generation == ref.generation);

        return &node.item;
    }

    T* get(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(!node.free);
        assert(node.generation == ref.generation);

        return &node.item;
    }

    const T* tryGet(Ref<T> ref) const {
        const ItemNode& node = nodes[ref.index];

        if (!node.free && node.generation == ref.generation) {
            return &node.item;
        }
        else {
            return nullptr;
        }
    }

    T* tryGet(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        if (!node.free && node.generation == ref.generation) {
            return &node.item;
        }
        else {
            return nullptr;
        }
    }

    void release(Ref<T> ref) {
        ItemNode& node = nodes[ref.index];

        assert(!node.free);
        assert(node.generation == ref.generation);

        node.free = 1;
        std::swap(node.nextIndex, firstAvailable);
        if constexpr (std::is_base_of<IDisposable, T>()) {
            node.item.dispose();
        }
        node.item.~T();

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
