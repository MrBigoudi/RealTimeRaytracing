#include "triangle.hpp"

Triangle::Triangle(uint32_t id, uint32_t materialId, const glm::vec4& p0, const glm::vec4& p1, const glm::vec4& p2){
    _InternalStruct = {};
    _InternalStruct._Id = id;
    _InternalStruct._MaterialId = materialId;
    _InternalStruct._P0 = p0;
    _InternalStruct._P1 = p1;
    _InternalStruct._P2 = p2;
}
