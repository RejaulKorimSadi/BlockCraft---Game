// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef BLOCKS_H
#define BLOCKS_H

#include <cstdint>

typedef uint8_t Block;

enum BlockType : Block {
    AIR = 0, GRASS, DIRT, STONE, SAND, LOG, LEAVES, PLANKS, COBBLE, GLASS, WATER, BEDROCK, BRICK,
    TALL_GRASS, ROSE, DANDELION, COAL_ORE, IRON_ORE, GOLD_ORE, DIAMOND_ORE,
    BLOCK_COUNT
};

// tile numbers inside the texture atlas
enum Tile {
    T_GRASS_TOP = 0, T_GRASS_SIDE, T_DIRT, T_STONE, T_SAND, T_LOG_SIDE, T_LOG_TOP, T_LEAVES,
    T_PLANKS, T_COBBLE, T_GLASS, T_WATER, T_BEDROCK, T_BRICK, T_TALL_GRASS, T_ROSE,
    T_DANDELION, T_COAL, T_IRON, T_GOLD, T_DIAMOND,
    TILE_COUNT
};

enum Face { F_TOP = 0, F_BOTTOM, F_SIDE };

inline int tileFor(Block b, Face f) {
    switch (b) {
    case GRASS:       return f == F_TOP ? T_GRASS_TOP : (f == F_BOTTOM ? T_DIRT : T_GRASS_SIDE);
    case DIRT:        return T_DIRT;
    case STONE:       return T_STONE;
    case SAND:        return T_SAND;
    case LOG:         return f == F_SIDE ? T_LOG_SIDE : T_LOG_TOP;
    case LEAVES:      return T_LEAVES;
    case PLANKS:      return T_PLANKS;
    case COBBLE:      return T_COBBLE;
    case GLASS:       return T_GLASS;
    case WATER:       return T_WATER;
    case BEDROCK:     return T_BEDROCK;
    case BRICK:       return T_BRICK;
    case TALL_GRASS:  return T_TALL_GRASS;
    case ROSE:        return T_ROSE;
    case DANDELION:   return T_DANDELION;
    case COAL_ORE:    return T_COAL;
    case IRON_ORE:    return T_IRON;
    case GOLD_ORE:    return T_GOLD;
    case DIAMOND_ORE: return T_DIAMOND;
    default:          return T_DIRT;
    }
}

inline const char* blockName(Block b) {
    static const char* names[] = {"Air", "Grass", "Dirt", "Stone", "Sand", "Oak Log", "Leaves", "Planks",
                                  "Cobblestone", "Glass", "Water", "Bedrock", "Bricks", "Tall Grass", "Rose",
                                  "Dandelion", "Coal Ore", "Iron Ore", "Gold Ore", "Diamond Ore"};
    return b < BLOCK_COUNT ? names[b] : "?";
}

// plants are drawn as two crossed pictures instead of a cube
inline bool isPlant(Block b) { return b == TALL_GRASS || b == ROSE || b == DANDELION; }

inline bool isSolid(Block b) { return b != AIR && b != WATER && !isPlant(b); }

// see-through blocks still let you see the faces of blocks behind them
inline bool isTransparent(Block b) { return b == AIR || b == LEAVES || b == GLASS || b == WATER || isPlant(b); }

// blocks that darken the corners next to them (ambient occlusion)
inline bool castsShadow(Block b) { return isSolid(b) && b != GLASS; }

#endif
