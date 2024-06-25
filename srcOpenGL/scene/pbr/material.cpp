#include "material.hpp"

#include <cstdlib>

uint32_t Material::_IdGenerator = 0;


Material::Material(const glm::vec4& color){
    _Id = _IdGenerator++;
    _InternalStruct = {};
    _InternalStruct._Color = color;
}

Material::Material(){
    _Id = _IdGenerator++;
    _InternalStruct = {};
    _InternalStruct._Color = glm::vec4(
        static_cast<float>(rand()) / RAND_MAX, 
        static_cast<float>(rand()) / RAND_MAX, 
        static_cast<float>(rand()) / RAND_MAX, 
        1.f
    );
}