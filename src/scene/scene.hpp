#pragma once

#include <vector>
#include <array>

#include "triangle.hpp"
#include "material.hpp"

class Scene{
    private:
        std::vector<Triangle> _Triangles = {};
        std::vector<Material> _Materials = {};

    public:
        Scene();

    public:
        std::array<TriangleGPU, MAX_NB_TRIANGLES> getTriangleToGPUData() const;
        std::array<MaterialGPU, MAX_NB_MATERIALS> getMaterialToGPUData() const;
};