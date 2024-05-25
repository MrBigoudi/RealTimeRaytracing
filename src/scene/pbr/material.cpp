#include "material.hpp"

#include <cstdlib>

Material::Material(uint32_t id, const glm::vec4& color){
    _InternalStruct = {};
    _InternalStruct._Id = id;
    _InternalStruct._Color = color;
}

Material::Material(uint32_t id){
    _InternalStruct = {};
    _InternalStruct._Id = id;
    _InternalStruct._Color = glm::vec4(
        static_cast<float>(rand()) / RAND_MAX, 
        static_cast<float>(rand()) / RAND_MAX, 
        static_cast<float>(rand()) / RAND_MAX, 
        1.f
    );
}