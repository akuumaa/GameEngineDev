/**
 * Copyright © 2026 Alexander Hancharenka
 * Copyright © 2026 Enrico Leidner
*/

#pragma once

#include <cmath>
#include <cstdint>

namespace Engine::Math
{
    class Noise
    {
        public:
            explicit Noise(uint32_t seed = 0) : seed(seed) {}

            double sample(double x, double y) const
            {
                double fx = std::floor(x), fy = std::floor(y);
                int32_t ix = (int32_t)fx, iy = (int32_t)fy;
                double tx = fade(x - fx), ty = fade(y - fy);

                double a = lattice(ix,     iy);
                double b = lattice(ix + 1, iy);
                double c = lattice(ix,     iy + 1);
                double d = lattice(ix + 1, iy + 1);

                double top    = a + (b - a) * tx;
                double bottom = c + (d - c) * tx;
                return top + (bottom - top) * ty;
            }

            double fractal(double x, double y, int octaves = 4,
                        double lacunarity = 2.0, double gain = 0.5) const
            {
                double sum = 0.0, amplitude = 1.0, total = 0.0;
                for (int i = 0; i < octaves; i++)
                {
                    sum += sample(x, y) * amplitude;
                    total += amplitude;
                    x *= lacunarity; y *= lacunarity;
                    amplitude *= gain;
                }
                return sum / total;
            }
        
        private:
            double lattice(int32_t ix, int32_t iy) const
            {
                uint32_t h = (uint32_t)ix * 0x8da6b343u
                           ^ (uint32_t)iy * 0xd8163841u
                           ^ seed * 0xcb1ab31fu;
                h ^= h >> 16; h *= 0x85ebca6bu;
                h ^= h >> 13; h *= 0xc2b2ae35u;
                h ^= h >> 16;
                return h / 4294967295.0;
            }

            static double fade(double t)
            {
                return t * t * (3.0 - 2.0 * t);
            }

            uint32_t seed = 0;
    };
}
