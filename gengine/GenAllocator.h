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

    struct Indices {
        uint32_t nextIndex;
        uint32_t generation;
    };

    // Note: the following doesn't work because of some undefined behavior? Need to examine this later.
    // std::vector<std::aligned_storage_t<sizeof(T), alignof(T)>> data;
    std::vector<T> data;
    std::vector<Indices> indices;

    uint32_t count;
    uint32_t capacity;

    uint32_t firstAvailable;

    GenAllocator(uint32_t capacity = 0) :
        count(0), capacity(capacity), data(capacity), indices(capacity), firstAvailable(0)
    {
        for (uint32_t i = 0; i < capacity; ++i) {
            indices[i].nextIndex = i + 1;
            indices[i].generation = 0;
        }
    }

    void dispose() {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (indices[i].generation == 0) {
                // This doesn't work in C++14 because of the lack of constexpr if
                // So we need to manually free resources before deleting allocator
                IF_CONSTEXPR(std::is_base_of<IDisposable, T>()) {
                    reinterpret_cast<T*>(&data[i])->dispose();
                }
            }
        }
    }

    void expand(uint32_t newCapacity) {
        assert (newCapacity >= capacity);

        data.resize(newCapacity);
        indices.resize(newCapacity);

        for (uint32_t i = capacity; i < newCapacity; ++i) {
            indices[i].nextIndex = i + 1;
            indices[i].generation = 0;
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
        data[newIndex] = T(std::forward<Args>(args)...);
        auto& newIndices = indices[newIndex];
        firstAvailable = newIndices.nextIndex;

        newIndices.generation++;

        count++;

        // also return the reference object of the resource
        return Ref<T> {newIndex, newIndices.generation};
    }

    Ref<T> clone(Ref<T> ref) {
        Ref<T> newRef = make();
        data[newRef.index] = data[ref.index];
        return newRef;
    }

    bool has(Ref<T> ref) const {
        auto idx = indices[ref.index];
        return idx.generation != 0 && idx.generation == ref.generation;
    }

    const T* get(Ref<T> ref) const {
        auto idx = indices[ref.index];

        assert(idx.generation != 0);
        assert(idx.generation == ref.generation);

        return reinterpret_cast<T*>(&data[ref.index]);
    }

    T* get(Ref<T> ref) {
        auto idx = indices[ref.index];

        assert(idx.generation != 0);
        assert(idx.generation == ref.generation);

        return reinterpret_cast<T*>(&data[ref.index]);
    }

    const T* tryGet(Ref<T> ref) const {
        auto idx = indices[ref.index];

        if (idx.generation != 0 && idx.generation == ref.generation) {
            return reinterpret_cast<T*>(&data[ref.index]);
        }
        else {
            return nullptr;
        }
    }

    T* tryGet(Ref<T> ref) {
        auto idx = indices[ref.index];

        if (idx.generation != 0 && idx.generation == ref.generation) {
            return &data[ref.index];
        }
        else {
            return nullptr;
        }
    }

    void release(Ref<T> ref) {
        auto idx = indices[ref.index];
        assert(idx.generation != 0);
        assert(idx.generation == ref.generation);

        indices[ref.index].nextIndex = firstAvailable;
        firstAvailable = ref.index;

        count--;
    }

    template <class Fun>
    void forEach(Fun&& fun) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (indices[i].generation != 0) {
                Ref<T> ref = {i, indices[i].generation};
                fun(data[i], ref);
            }
        }
    }

    template <class Fun>
    void forEachUntil(Fun&& fun) {
        for (uint32_t i = 0; i < capacity; ++i) {
            if (indices[i].generation != 0) {
                Ref<T> ref = {i, indices[i].generation};
                bool end = fun(data[i], ref);
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

    template <typename T>
    static Ref<T> clone(Ref<T> ref) {
        return getStorage<T>().clone(ref);
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
