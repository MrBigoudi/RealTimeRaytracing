#pragma once

#include <vector>
#include <array>

#include "program.hpp"
#include "triangle.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include <glad/gl.h>

#include <memory>

class Scene;
using ScenePtr = std::shared_ptr<Scene>;

class Scene{
    private:
        std::vector<Material> _Materials = {Material(0, {1.f,1.f,1.f,1.f})}; // always one default material
        std::vector<MeshPtr> _Meshes = {};

        uint32_t _MaterialIdGenerator = 0;
        
        GLuint _TrianglesSSBO = 0;
        GLuint _MaterialsSSBO = 0;
        GLuint _MeshModelsSSBO = 0;

        uint32_t _NbTriangles = 0;
        uint32_t _NbMaterials = 1; // the default one
        uint32_t _NbMeshes = 0;

    public:
        Scene();

    public:
        std::array<TriangleGPU, MAX_NB_TRIANGLES> getTriangleToGPUData() const;
        std::array<MaterialGPU, MAX_NB_MATERIALS> getMaterialToGPUData() const;
        std::array<MeshModelGPU, MAX_NB_MESHES> getMeshModelToGPUData() const;

        void addMesh(MeshPtr mesh);
        void addMaterial(const glm::vec4& color);
        void addRandomMaterial();

        void sendDataToGpu(ProgramPtr program);

    private:
        void createSSBO();
        void bindSSBO();
};