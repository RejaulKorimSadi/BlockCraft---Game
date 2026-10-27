// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef WORLD_H
#define WORLD_H

#include "blocks.h"
#include <GL/freeglut.h>
#include <vector>
#include <string>
#include <cstdint>

const int WORLD_X = 192;
const int WORLD_Z = 192;
const int WORLD_Y = 64;
const int CHUNK = 16;
const int CHUNKS_X = WORLD_X / CHUNK;
const int CHUNKS_Z = WORLD_Z / CHUNK;
const int SEA_LEVEL = 22;

struct RayHit {
    bool hit = false;
    int x = 0, y = 0, z = 0;      // the block we are looking at
    int px = 0, py = 0, pz = 0;   // the empty cell in front of it, where a new block goes
};

class World {
public:
    explicit World(uint32_t seed);
    ~World();

    void generate();
    Block get(int x, int y, int z) const;
    void set(int x, int y, int z, Block b);
    int surfaceHeight(int x, int z) const;

    void buildAllMeshes();
    void drawOpaque(float px, float pz) const;
    void drawWater(float px, float pz) const;

    void rebuildDirty();
    RayHit raycast(float ox, float oy, float oz, float dx, float dy, float dz, float maxDist) const;

    bool save(const std::string& file, const float* player) const;
    bool load(const std::string& file, float* player);

    float skyLight(int x, int y, int z) const;

    uint32_t seed;
    float drawRange = 115.0f;   // chunks farther than this are hidden by fog anyway

private:
    std::vector<Block> blocks;
    GLuint opaqueLists = 0, waterLists = 0;
    std::vector<bool> dirty;
    std::vector<int> tops;   // highest ground block in each column, for darkening caves

    int index(int x, int y, int z) const { return (y * WORLD_Z + z) * WORLD_X + x; }
    void placeTree(int x, int y, int z, float r);
    void buildChunk(int cx, int cz);
    void markDirty(int x, int z);
    bool faceVisible(Block self, Block next) const;
    void emitFace(int x, int y, int z, int face, Block b) const;
    void emitPlant(int x, int y, int z, Block b) const;
    int cornerAO(int x, int y, int z, int face, const int* corner) const;
    bool chunkInRange(int i, float px, float pz) const;
    void refreshTops();
};

#endif
