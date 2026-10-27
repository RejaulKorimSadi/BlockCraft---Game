// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
// Checks that run without opening a window: same seed gives the same world, ray casting and saving work.
#include "../lib/world.h"
#include <cstdio>
#include <cmath>

static int failures = 0;

static void check(bool ok, const char* what) {
    std::printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) failures++;
}

// FNV-1a over every block, a fingerprint of the whole world
static uint32_t fingerprint(const World& w) {
    uint32_t h = 2166136261u;
    for (int y = 0; y < WORLD_Y; y++)
        for (int z = 0; z < WORLD_Z; z++)
            for (int x = 0; x < WORLD_X; x++) {
                h ^= w.get(x, y, z);
                h *= 16777619u;
            }
    return h;
}

int main() {
    const uint32_t SEED = 20261028u;
    const uint32_t REFERENCE = 0xE8616371u;   // seed 20261028 on GCC 13, x86-64
    std::printf("BlockCraft world tests (seed %u)\n", SEED);

    World a(SEED), b(SEED), c(SEED + 1);
    a.generate();
    b.generate();
    c.generate();
    uint32_t fa = fingerprint(a), fb = fingerprint(b), fc = fingerprint(c);
    std::printf("  world fingerprint: %08X\n", fa);
    check(fa == fb, "the same seed builds the same world twice");
    check(fa != fc, "a different seed builds a different world");

    long count[BLOCK_COUNT] = {0};
    long underground = 0, caveAir = 0;
    for (int x = 0; x < WORLD_X; x++)
        for (int z = 0; z < WORLD_Z; z++) {
            int top = a.surfaceHeight(x, z);
            for (int y = 0; y < WORLD_Y; y++) {
                Block blk = a.get(x, y, z);
                count[blk]++;
                if (y > 0 && y < top - 4) { underground++; if (blk == AIR) caveAir++; }
            }
        }
    std::printf("  water %ld, trees %ld logs, plants %ld, ores c/i/g/d %ld/%ld/%ld/%ld, caves %.1f%% of underground\n",
                count[WATER], count[LOG], count[TALL_GRASS] + count[ROSE] + count[DANDELION], count[COAL_ORE],
                count[IRON_ORE], count[GOLD_ORE], count[DIAMOND_ORE], 100.0 * caveAir / underground);
    check(count[WATER] > 0 && count[LOG] > 0 && count[GRASS] > 0, "terrain has water, trees and grass");
    check(count[DIAMOND_ORE] > 0 && count[COAL_ORE] > count[DIAMOND_ORE], "ores exist and diamonds are rarer than coal");
    check(caveAir > 0 && caveAir < underground / 4, "caves exist but do not hollow out the ground");

    check(a.skyLight(WORLD_X / 2, WORLD_Y - 1, WORLD_Z / 2) == 1.0f && a.skyLight(WORLD_X / 2, 3, WORLD_Z / 2) < 1.0f,
          "the sky is bright and deep underground is darker");

    bool bedrock = true;
    for (int x = 0; x < WORLD_X; x++)
        for (int z = 0; z < WORLD_Z; z++)
            if (a.get(x, 0, z) != BEDROCK) bedrock = false;
    check(bedrock, "the bottom layer is all bedrock");

    int sx = WORLD_X / 2, sz = WORLD_Z / 2, sy = a.surfaceHeight(sx, sz);
    RayHit hit = a.raycast(sx + 0.5f, sy + 5.5f, sz + 0.5f, 0, -1, 0, 10);
    check(hit.hit && hit.x == sx && hit.z == sz && hit.y == sy, "a ray straight down hits the surface block");
    check(hit.py == sy + 1, "the cell in front of the hit is the one above it");

    float player[5] = {1.5f, 40.0f, 2.5f, 90.0f, -10.0f}, back[5];
    a.set(sx, sy + 1, sz, BRICK);
    bool saved = a.save("test_world.dat", player);
    World d(1);
    bool loaded = d.load("test_world.dat", back);
    check(saved && loaded, "the world saves and loads");
    check(fingerprint(d) == fingerprint(a) && d.seed == SEED, "the loaded world matches block for block");
    check(back[0] == player[0] && back[4] == player[4], "the player position comes back too");
    std::remove("test_world.dat");

    // a different compiler may round a float differently, so this is reported, not failed
    std::printf("  reference fingerprint %08X: %s\n", REFERENCE, fa == REFERENCE ? "matches" : "differs on this compiler");

    std::printf(failures ? "%d check(s) failed\n" : "all checks passed\n", failures);
    return failures ? 1 : 0;
}
