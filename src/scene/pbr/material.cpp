#include "material.hpp"

Material::Material(uint32_t id, const glm::vec4& color){
    _InternalStruct = {};
    _InternalStruct._Id = id;
    _InternalStruct._Color = color;
}
