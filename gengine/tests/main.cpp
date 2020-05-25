//
// Created by lasagnaphil on 20. 1. 29..
//

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "gengine/Arena.h"

struct alignas(16) Obj {
    int a, b, c, d;
    float x, y;

    Obj(int n = 0) : a(n), b(n), c(n), d(n), x((float)n), y((float)n) {}

    bool operator==(const Obj& other) const {
        return a == other.a && b == other.b && c == other.c && d == other.d
                && x == other.x && y == other.y;
    }

    bool operator!=(const Obj& other) const {
        return !(*this == other);
    }
};

TEST_CASE("Testing Arena") {
    SUBCASE("Test default initialization") {
        Arena<Obj> arena;
        REQUIRE(arena.size() == 0);
        REQUIRE(arena.capacity() == 0);
    }
    SUBCASE("Test initialization with capacity") {
        Arena<Obj> arena(16);
        REQUIRE(arena.size() == 0);
        REQUIRE(arena.capacity() == 16);
    }

    SUBCASE("Simple test") {
        Arena<Obj> arena;

        Ref<Obj> a = arena.make(1);
        REQUIRE(arena.size() == 1);
        Ref<Obj> b = arena.make(2);
        REQUIRE(arena.size() == 2);
        Ref<Obj> c = arena.make();
        *arena.get(c) = 3;
        REQUIRE(arena.size() == 3);

        REQUIRE(*arena.get(a) == 1);
        REQUIRE(*arena.get(b) == 2);
        REQUIRE(*arena.get(c) == 3);
        REQUIRE(arena.has(a));
        REQUIRE(arena.has(b));
        REQUIRE(arena.has(c));

        REQUIRE(!arena.has(Ref<Obj>{a.index, a.generation + 1}));
        REQUIRE(!arena.has(Ref<Obj>{a.index, a.generation - 1}));

        arena.release(a);
        REQUIRE(!arena.has(a));
        REQUIRE(arena.has(b));
        REQUIRE(arena.has(c));
        REQUIRE(arena.size() == 2);

        arena.release(c);
        REQUIRE(!arena.has(a));
        REQUIRE(arena.has(b));
        REQUIRE(!arena.has(c));
        REQUIRE(arena.size() == 1);

        arena.release(b);
        REQUIRE(!arena.has(a));
        REQUIRE(!arena.has(b));
        REQUIRE(!arena.has(c));
        REQUIRE(arena.size() == 0);
    }

    auto shuffle = [](int* arr, size_t n) {
        if (n > 1) {
            size_t i;
            srand(time(NULL));
            for (i = 0; i < n - 1; i++) {
                size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
                int t = arr[j];
                arr[j] = arr[i];
                arr[i] = t;
            }
        }
    };

    SUBCASE("Complex test") {
        int testSize = 2048;
        auto arena = Arena<Obj>(testSize);
        std::vector<Ref<Obj>> refs(testSize);
        for (int i = 0; i < testSize; i++) {
            refs[i] = arena.make(i);
        }
        REQUIRE(arena.size() == testSize);
        int deleteSize = 1000;
        std::vector<int> shuffled(testSize);
        for (int i = 0; i < testSize; i++) {
            shuffled[i] = i;
        }
        shuffle(shuffled.data(), shuffled.size());
        for (int i = 0; i < deleteSize; i++) {
            arena.release(refs[shuffled[i]]);
        }
        REQUIRE(arena.size() == testSize - deleteSize);

        for (int i = 0; i < deleteSize; i++) {
            REQUIRE(!arena.has(refs[shuffled[i]]));
        }
        for (int i = deleteSize; i < testSize; i++) {
            REQUIRE(arena.has(refs[shuffled[i]]));
            REQUIRE(*arena.get(refs[shuffled[i]]) == shuffled[i]);
        }

        std::vector<Ref<Obj>> newRefs(deleteSize);
        for (int i = 0; i < deleteSize; i++) {
            newRefs[i] = arena.make(shuffled[i]);
        }
        REQUIRE(arena.size() == testSize);
        for (int i = 0; i < deleteSize; i++) {
            REQUIRE(arena.has(newRefs[i]));
            REQUIRE(*arena.get(newRefs[i]) == shuffled[i]);
        }
        for (int i = deleteSize; i < testSize; i++) {
            REQUIRE(arena.has(refs[shuffled[i]]));
            REQUIRE(*arena.get(refs[shuffled[i]]) == shuffled[i]);
        }
    }
}

