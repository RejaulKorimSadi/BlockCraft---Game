// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#include "world.h"
#include "noise.h"
#include "textures.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <cstring>

World::World(uint32_t s)
    : seed(s), blocks(WORLD_X * WORLD_Y * WORLD_Z, AIR), dirty(CHUNKS_X * CHUNKS_Z, true), tops(WORLD_X * WORLD_Z, 0) {}

World::~World() {
    if (opaqueLists) glDeleteLists(opaqueLists, CHUNKS_X * CHUNKS_Z);
    if (waterLists) glDeleteLists(waterLists, CHUNKS_X * CHUNKS_Z);
}

Block World::get(int x, int y, int z) const {
    if (x < 0 || z < 0 || x >= WORLD_X || z >= WORLD_Z || y >= WORLD_Y) return AIR;
    if (y < 0) return BEDROCK;
    return blocks[index(x, y, z)];
}

void World::set(int x, int y, int z, Block b) {
    if (x < 0 || z < 0 || y < 0 || x >= WORLD_X || z >= WORLD_Z || y >= WORLD_Y) return;
    blocks[index(x, y, z)] = b;
    tops[z * WORLD_X + x] = surfaceHeight(x, z);
    markDirty(x, z);
    if (!isSolid(b) && isPlant(get(x, y + 1, z))) set(x, y + 1, z, AIR);   // a flower can't float
}

void World::markDirty(int x, int z) {
    // neighbours matter too: their faces and corner shadows depend on this block
    for (int dx = -1; dx <= 1; dx++)
        for (int dz = -1; dz <= 1; dz++) {
            int nx = x + dx, nz = z + dz;
            if (nx < 0 || nz < 0 || nx >= WORLD_X || nz >= WORLD_Z) continue;
            dirty[(nz / CHUNK) * CHUNKS_X + nx / CHUNK] = true;
        }
}

int World::surfaceHeight(int x, int z) const {
    for (int y = WORLD_Y - 1; y > 0; y--) {
        Block b = get(x, y, z);
        if (b != AIR && b != LEAVES && !isPlant(b)) return y;
    }
    return 0;
}

void World::generate() {
    std::fill(blocks.begin(), blocks.end(), AIR);
    Noise hills(seed), detail(seed * 7 + 3), trees(seed * 13 + 5);
    Noise caveA(seed * 31 + 7), caveB(seed * 37 + 11), rooms(seed * 41 + 13), ores(seed * 43 + 17);

    for (int x = 0; x < WORLD_X; x++) {
        for (int z = 0; z < WORLD_Z; z++) {
            float h = hills.fractal(x / 48.0f, z / 48.0f, 4);
            float d = detail.fractal(x / 12.0f, z / 12.0f, 2);
            int height = 12 + (int)(h * 26.0f + d * 4.0f);
            if (height >= WORLD_Y - 8) height = WORLD_Y - 8;
            bool dry = height > SEA_LEVEL + 3;

            for (int y = 0; y <= height; y++) {
                Block b;
                if (y == 0) b = BEDROCK;
                else if (y < height - 3) b = STONE;
                else if (height <= SEA_LEVEL + 1) b = SAND;
                else b = (y == height) ? GRASS : DIRT;

                if (y >= 2) {
                    // two noise fields that are both close to 0.5 meet along thin winding lines: tunnels
                    float a = caveA.fractal3(x / 22.0f, y / 14.0f, z / 22.0f), c = caveB.fractal3(x / 22.0f, y / 14.0f, z / 22.0f);
                    bool tunnel = std::fabs(a - 0.5f) < 0.035f && std::fabs(c - 0.5f) < 0.035f && (y <= height - 4 || dry);
                    bool room = y <= height - 5 && rooms.fractal3(x / 14.0f, y / 9.0f, z / 14.0f) > 0.74f;
                    if (tunnel || room) b = AIR;
                }
                if (b == STONE) {
                    float r = ores.hash3(x, y, z);
                    if (y < 12 && r < 0.0025f) b = DIAMOND_ORE;
                    else if (y < 20 && r < 0.006f) b = GOLD_ORE;
                    else if (y < 34 && r < 0.014f) b = IRON_ORE;
                    else if (r < 0.034f) b = COAL_ORE;
                }
                blocks[index(x, y, z)] = b;
            }
            for (int y = height + 1; y <= SEA_LEVEL; y++) blocks[index(x, y, z)] = WATER;
        }
    }

    for (int x = 3; x < WORLD_X - 3; x++) {
        for (int z = 3; z < WORLD_Z - 3; z++) {
            float r = trees.hash(x, z);
            if (r < 0.985f) continue;
            int y = surfaceHeight(x, z);
            if (get(x, y, z) == GRASS) placeTree(x, y + 1, z, trees.hash(z, x));
        }
    }

    for (int x = 0; x < WORLD_X; x++) {
        for (int z = 0; z < WORLD_Z; z++) {
            int y = surfaceHeight(x, z);
            if (get(x, y, z) != GRASS || get(x, y + 1, z) != AIR) continue;
            float p = detail.hash(x * 3 + 1, z * 5 + 2);
            if (p > 0.86f) blocks[index(x, y + 1, z)] = TALL_GRASS;
            else if (p > 0.845f) blocks[index(x, y + 1, z)] = ROSE;
            else if (p > 0.83f) blocks[index(x, y + 1, z)] = DANDELION;
        }
    }
    refreshTops();
    std::fill(dirty.begin(), dirty.end(), true);
}

void World::placeTree(int x, int y, int z, float r) {
    int trunk = 4 + (int)(r * 3.0f);
    if (y + trunk + 2 >= WORLD_Y) return;
    for (int dy = trunk - 2; dy <= trunk + 1; dy++) {
        int rad = dy >= trunk ? 1 : 2;
        for (int dx = -rad; dx <= rad; dx++)
            for (int dz = -rad; dz <= rad; dz++) {
                if (rad == 2 && std::abs(dx) == 2 && std::abs(dz) == 2) continue;   // round off the corners
                if (get(x + dx, y + dy, z + dz) == AIR) blocks[index(x + dx, y + dy, z + dz)] = LEAVES;
            }
    }
    for (int dy = 0; dy < trunk; dy++) blocks[index(x, y + dy, z)] = LOG;
}

void World::refreshTops() {
    for (int x = 0; x < WORLD_X; x++)
        for (int z = 0; z < WORLD_Z; z++) tops[z * WORLD_X + x] = surfaceHeight(x, z);
}

// the deeper a face is below the ground of its column, the less daylight reaches it
float World::skyLight(int x, int y, int z) const {
    if (x < 0 || z < 0 || x >= WORLD_X || z >= WORLD_Z) return 1.0f;
    int depth = tops[z * WORLD_X + x] - y;
    if (depth <= 0) return 1.0f;
    return std::max(0.42f, 1.0f - depth * 0.07f);
}

// shading per face direction fakes a sun shining from above
static const float SHADE[6] = {1.0f, 0.55f, 0.8f, 0.8f, 0.68f, 0.68f};

// how bright a corner is when 0, 1, 2 or 3 of the blocks around it are solid
static const float AO_LIGHT[4] = {0.45f, 0.64f, 0.82f, 1.0f};

static const int NORMAL[6][3] = {{0, 1, 0}, {0, -1, 0}, {1, 0, 0}, {-1, 0, 0}, {0, 0, 1}, {0, 0, -1}};

// the four corners of each face, counter-clockwise seen from outside, plus which side of the tile they use
static const int CORNER[6][4][3] = {
    {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}},
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}},
    {{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}},
    {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}},
    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}},
    {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}},
};
static const int CORNER_UV[6][4][2] = {
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}},
    {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}},
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}},
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}},
    {{0, 1}, {1, 1}, {1, 0}, {0, 0}},
};

bool World::faceVisible(Block self, Block next) const {
    if (next == AIR) return true;
    if (self == WATER) return false;
    if (isTransparent(next)) return next != self || self == LEAVES;
    return false;
}

// the "0fps" trick: look at the two side blocks and the corner block next to a vertex
int World::cornerAO(int x, int y, int z, int face, const int* c) const {
    int n[3] = {NORMAL[face][0], NORMAL[face][1], NORMAL[face][2]};
    int s1[3] = {0, 0, 0}, s2[3] = {0, 0, 0};
    int axis = n[0] ? 0 : (n[1] ? 1 : 2);
    int a = (axis + 1) % 3, b = (axis + 2) % 3;
    s1[a] = c[a] ? 1 : -1;
    s2[b] = c[b] ? 1 : -1;
    int bx = x + n[0], by = y + n[1], bz = z + n[2];
    bool side1 = castsShadow(get(bx + s1[0], by + s1[1], bz + s1[2]));
    bool side2 = castsShadow(get(bx + s2[0], by + s2[1], bz + s2[2]));
    bool corner = castsShadow(get(bx + s1[0] + s2[0], by + s1[1] + s2[1], bz + s1[2] + s2[2]));
    if (side1 && side2) return 0;
    return 3 - (side1 + side2 + corner);
}

void World::emitFace(int x, int y, int z, int face, Block b) const {
    Face kind = face == 0 ? F_TOP : (face == 1 ? F_BOTTOM : F_SIDE);
    float uv[4];
    tileUV(tileFor(b, kind), uv[0], uv[1], uv[2], uv[3]);

    float top = 1.0f;
    if (b == WATER && get(x, y + 1, z) != WATER) top = 0.88f;

    float sky = b == WATER ? 1.0f : skyLight(x + NORMAL[face][0], y + NORMAL[face][1], z + NORMAL[face][2]);
    int ao[4];
    for (int i = 0; i < 4; i++) ao[i] = b == WATER ? 3 : cornerAO(x, y, z, face, CORNER[face][i]);

    // split the square along the brighter diagonal, otherwise the shadow smears in one direction
    static const int A[6] = {0, 1, 2, 0, 2, 3}, B[6] = {1, 2, 3, 1, 3, 0};
    const int* order = (ao[0] + ao[2] < ao[1] + ao[3]) ? B : A;

    for (int k = 0; k < 6; k++) {
        int i = order[k];
        const int* c = CORNER[face][i];
        float l = SHADE[face] * AO_LIGHT[ao[i]] * sky;
        glColor3f(l, l, l);
        glTexCoord2f(uv[CORNER_UV[face][i][0] ? 2 : 0], uv[CORNER_UV[face][i][1] ? 3 : 1]);
        glVertex3f((float)(x + c[0]), y + (c[1] ? top : 0.0f), (float)(z + c[2]));
    }
}

// flowers and grass: two pictures crossing in an X, each drawn from both sides
void World::emitPlant(int x, int y, int z, Block b) const {
    float u0, v0, u1, v1;
    tileUV(tileFor(b, F_SIDE), u0, v0, u1, v1);
    float e = 0.15f, X0 = x + e, X1 = x + 1 - e, Z0 = z + e, Z1 = z + 1 - e, Y0 = (float)y, Y1 = y + 0.9f;
    float quads[2][4] = {{X0, Z0, X1, Z1}, {X0, Z1, X1, Z0}};
    glColor3f(0.92f, 0.92f, 0.92f);
    for (int q = 0; q < 2; q++)
        for (int side = 0; side < 2; side++) {
            float ax = quads[q][side ? 2 : 0], az = quads[q][side ? 3 : 1];
            float bx = quads[q][side ? 0 : 2], bz = quads[q][side ? 1 : 3];
            float ua = side ? u1 : u0, ub = side ? u0 : u1;
            glTexCoord2f(ua, v1); glVertex3f(ax, Y0, az);
            glTexCoord2f(ub, v1); glVertex3f(bx, Y0, bz);
            glTexCoord2f(ub, v0); glVertex3f(bx, Y1, bz);
            glTexCoord2f(ua, v1); glVertex3f(ax, Y0, az);
            glTexCoord2f(ub, v0); glVertex3f(bx, Y1, bz);
            glTexCoord2f(ua, v0); glVertex3f(ax, Y1, az);
        }
}

void World::buildChunk(int cx, int cz) {
    int id = cz * CHUNKS_X + cx;

    for (int pass = 0; pass < 2; pass++) {
        // pass 0 = everything solid, pass 1 = water, which is drawn later with blending
        glNewList((pass == 0 ? opaqueLists : waterLists) + id, GL_COMPILE);
        glBegin(GL_TRIANGLES);
        for (int y = 0; y < WORLD_Y; y++)
            for (int z = cz * CHUNK; z < (cz + 1) * CHUNK; z++)
                for (int x = cx * CHUNK; x < (cx + 1) * CHUNK; x++) {
                    Block b = blocks[index(x, y, z)];
                    if (b == AIR || (pass == 1) != (b == WATER)) continue;
                    if (isPlant(b)) { emitPlant(x, y, z, b); continue; }
                    for (int f = 0; f < 6; f++) {
                        if (y + NORMAL[f][1] < 0) continue;
                        Block n = get(x + NORMAL[f][0], y + NORMAL[f][1], z + NORMAL[f][2]);
                        if (faceVisible(b, n)) emitFace(x, y, z, f, b);
                    }
                }
        glEnd();
        glEndList();
    }
    dirty[id] = false;
}

void World::buildAllMeshes() {
    if (!opaqueLists) {
        opaqueLists = glGenLists(CHUNKS_X * CHUNKS_Z);
        waterLists = glGenLists(CHUNKS_X * CHUNKS_Z);
    }
    for (int cz = 0; cz < CHUNKS_Z; cz++)
        for (int cx = 0; cx < CHUNKS_X; cx++) buildChunk(cx, cz);
}

void World::rebuildDirty() {
    for (int cz = 0; cz < CHUNKS_Z; cz++)
        for (int cx = 0; cx < CHUNKS_X; cx++)
            if (dirty[cz * CHUNKS_X + cx]) buildChunk(cx, cz);
}

// chunks hidden by the fog are not worth sending to the graphics card
bool World::chunkInRange(int i, float px, float pz) const {
    float cx = (i % CHUNKS_X) * CHUNK + CHUNK / 2.0f, cz = (i / CHUNKS_X) * CHUNK + CHUNK / 2.0f;
    float dx = cx - px, dz = cz - pz;
    return dx * dx + dz * dz < drawRange * drawRange;
}

void World::drawOpaque(float px, float pz) const {
    for (int i = 0; i < CHUNKS_X * CHUNKS_Z; i++)
        if (chunkInRange(i, px, pz)) glCallList(opaqueLists + i);
}

void World::drawWater(float px, float pz) const {
    for (int i = 0; i < CHUNKS_X * CHUNKS_Z; i++)
        if (chunkInRange(i, px, pz)) glCallList(waterLists + i);
}

// walks the grid one cell at a time along the ray (the Amanatides & Woo method)
RayHit World::raycast(float ox, float oy, float oz, float dx, float dy, float dz, float maxDist) const {
    RayHit r;
    int x = (int)std::floor(ox), y = (int)std::floor(oy), z = (int)std::floor(oz);
    int sx = dx > 0 ? 1 : -1, sy = dy > 0 ? 1 : -1, sz = dz > 0 ? 1 : -1;
    float inf = 1e30f;
    float tdx = dx != 0 ? std::fabs(1.0f / dx) : inf;
    float tdy = dy != 0 ? std::fabs(1.0f / dy) : inf;
    float tdz = dz != 0 ? std::fabs(1.0f / dz) : inf;
    float tx = dx != 0 ? ((sx > 0 ? (x + 1 - ox) : (ox - x)) * tdx) : inf;
    float ty = dy != 0 ? ((sy > 0 ? (y + 1 - oy) : (oy - y)) * tdy) : inf;
    float tz = dz != 0 ? ((sz > 0 ? (z + 1 - oz) : (oz - z)) * tdz) : inf;

    int px = x, py = y, pz = z;
    float t = 0.0f;
    while (t <= maxDist) {
        Block b = get(x, y, z);
        if (b != AIR && b != WATER && y >= 0) {
            r.hit = true;
            r.x = x; r.y = y; r.z = z;
            r.px = px; r.py = py; r.pz = pz;
            return r;
        }
        px = x; py = y; pz = z;
        if (tx < ty && tx < tz) { x += sx; t = tx; tx += tdx; }
        else if (ty < tz) { y += sy; t = ty; ty += tdy; }
        else { z += sz; t = tz; tz += tdz; }
    }
    return r;
}

bool World::save(const std::string& file, const float* player) const {
    FILE* f = std::fopen(file.c_str(), "wb");
    if (!f) return false;
    int dims[3] = {WORLD_X, WORLD_Y, WORLD_Z};
    std::fwrite("BCW1", 1, 4, f);
    std::fwrite(&seed, sizeof(seed), 1, f);
    std::fwrite(dims, sizeof(int), 3, f);
    std::fwrite(player, sizeof(float), 5, f);
    std::fwrite(blocks.data(), 1, blocks.size(), f);
    std::fclose(f);
    return true;
}

bool World::load(const std::string& file, float* player) {
    FILE* f = std::fopen(file.c_str(), "rb");
    if (!f) return false;
    char magic[4];
    int dims[3];
    uint32_t s;
    bool ok = std::fread(magic, 1, 4, f) == 4 && std::memcmp(magic, "BCW1", 4) == 0 &&
              std::fread(&s, sizeof(s), 1, f) == 1 && std::fread(dims, sizeof(int), 3, f) == 3 &&
              dims[0] == WORLD_X && dims[1] == WORLD_Y && dims[2] == WORLD_Z;
    std::vector<Block> data(blocks.size());
    float p[5];
    ok = ok && std::fread(p, sizeof(float), 5, f) == 5 && std::fread(data.data(), 1, data.size(), f) == data.size();
    std::fclose(f);
    if (!ok) return false;
    seed = s;
    blocks.swap(data);
    refreshTops();
    std::memcpy(player, p, sizeof(p));
    std::fill(dirty.begin(), dirty.end(), true);
    return true;
}
