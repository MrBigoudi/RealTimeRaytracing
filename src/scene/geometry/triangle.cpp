#include "triangle.hpp"

#include <cstdlib>

uint32_t Triangle::_IdGenerator = 0;

Triangle::Triangle(const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2, uint32_t modelId, uint32_t materialId){
    _InternalStruct = {};
    _InternalStruct._Id = _IdGenerator++;
    _InternalStruct._MaterialId = materialId;
    _InternalStruct._ModelId = modelId;
    _InternalStruct._P0 = p0;
    _InternalStruct._P1 = p1;
    _InternalStruct._P2 = p2;
}

Triangle::Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, uint32_t modelId, uint32_t materialId){
    _InternalStruct = {};
    _InternalStruct._Id = _IdGenerator++;
    _InternalStruct._MaterialId = materialId;
    _InternalStruct._ModelId = modelId;
    _InternalStruct._P0 = glm::vec4(p0, 1.f);
    _InternalStruct._P1 = glm::vec4(p1, 1.f);
    _InternalStruct._P2 = glm::vec4(p2, 1.f);
}
