// BlockCraft by Md Rejaul Korim Sadi (231-115-349), CSE 402 CGIP Lab, Metropolitan University
#ifndef NOISE_H
#define NOISE_H

#include <cmath>
#include <cstdint>

// value noise: random numbers on an integer grid, smoothly blended in between
class Noise {
public:
    explicit Noise(uint32_t seed = 1337) : seed(seed) {}

    // several layers, each twice as detailed and half as strong as the one before
    float fractal(float x, float z, int octaves = 4) const {
        float total = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
        for (int i = 0; i < octaves; i++) {
            total += value(x * freq, z * freq) * amp;
            norm += amp;
            amp *= 0.5f;
            freq *= 2.0f;
        }
        return total / norm;
    }

    float fractal3(float x, float y, float z, int octaves = 2) const {
        float total = 0.0f, amp = 1.0f, freq = 1.0f, norm = 0.0f;
        for (int i = 0; i < octaves; i++) {
            total += value3(x * freq, y * freq, z * freq) * amp;
            norm += amp;
            amp *= 0.5f;
            freq *= 2.0f;
        }
        return total / norm;
    }

    // a stable pseudo random number in [0,1) for any integer cell
    float hash(int x, int z) const { return hash3(x, 0, z); }

    float hash3(int x, int y, int z) const {
        uint32_t h = seed;
        h ^= (uint32_t)x * 374761393u;
        h ^= (uint32_t)y * 2246822519u;
        h ^= (uint32_t)z * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;
        h ^= h >> 16;
        return (h & 0xFFFFFF) / 16777216.0f;
    }

private:
    uint32_t seed;

    static float smooth(float t) { return t * t * (3.0f - 2.0f * t); }
    static float lerp(float a, float b, float t) { return a + (b - a) * t; }

    float value(float x, float z) const {
        int x0 = (int)std::floor(x), z0 = (int)std::floor(z);
        float tx = smooth(x - x0), tz = smooth(z - z0);
        float top = lerp(hash(x0, z0), hash(x0 + 1, z0), tx);
        float bottom = lerp(hash(x0, z0 + 1), hash(x0 + 1, z0 + 1), tx);
        return lerp(top, bottom, tz);
    }

    float value3(float x, float y, float z) const {
        int x0 = (int)std::floor(x), y0 = (int)std::floor(y), z0 = (int)std::floor(z);
        float tx = smooth(x - x0), ty = smooth(y - y0), tz = smooth(z - z0);
        float a = lerp(hash3(x0, y0, z0), hash3(x0 + 1, y0, z0), tx);
        float b = lerp(hash3(x0, y0, z0 + 1), hash3(x0 + 1, y0, z0 + 1), tx);
        float c = lerp(hash3(x0, y0 + 1, z0), hash3(x0 + 1, y0 + 1, z0), tx);
        float d = lerp(hash3(x0, y0 + 1, z0 + 1), hash3(x0 + 1, y0 + 1, z0 + 1), tx);
        return lerp(lerp(a, b, tz), lerp(c, d, tz), ty);
    }
};

#endif
