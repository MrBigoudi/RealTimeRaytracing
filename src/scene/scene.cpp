#include "scene.hpp"

#include <algorithm>

Scene::Scene(){}

std::array<TriangleGPU, MAX_NB_TRIANGLES> Scene::getTriangleToGPUData() const {
    std::array<TriangleGPU, MAX_NB_TRIANGLES> trianglesGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Triangles.size()), MAX_NB_TRIANGLES); i++){
        trianglesGPU[i] = _Triangles[i]._InternalStruct;
    }
    return trianglesGPU;
}

std::array<MaterialGPU, MAX_NB_MATERIALS> Scene::getMaterialToGPUData() const {
    std::array<MaterialGPU, MAX_NB_MATERIALS> MaterialGPU{};
    for(int i=0; i<std::min(static_cast<int>(_Materials.size()), MAX_NB_MATERIALS); i++){
        MaterialGPU[i] = _Materials[i]._InternalStruct;
    }
    return MaterialGPU;
}