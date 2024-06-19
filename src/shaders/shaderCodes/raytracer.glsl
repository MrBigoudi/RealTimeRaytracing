#version 460 core

// structures
struct Camera {
    mat4 _View;
    mat4 _Proj;
    mat4 _InvView;
    mat4 _InvProj;
    vec4 _Eye;
    float _PlaneWidth;
    float _PlaneHeight;
    float _PlaneNear;
};

struct Ray {
    vec4 _Origin;
    vec4 _Direction;
};

struct Triangle {
    vec4 _P0;
    vec4 _P2;
    vec4 _P1;
    uint _ModelId;
};

struct Material {
    vec4 _Color;
};

struct Model {
    mat4 _ModelMatrix;
    uint _MaterialId;
};

struct Hit {
    vec4 _Coords; // (b0, b1, b2, t)
    uint _DidHit;
    uint _TriangleId;
};

struct AABB {
    vec3 _Min;
    vec3 _Max;
};

struct BVH_Node {
    AABB _BoundingBox;
    uint _TriangleId;
    uint _LeftChild;
    uint _RightChild;
};

// output
layout(rgba32f, binding = 0) uniform image2D oImage;

// input
layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

uniform float uTime;
uniform Camera uCamera;
uniform uint uNbMaterials;
uniform uint uNbTriangles;
uniform uint uNbModels;
uniform int uDepthDisplayBVH;
uniform bool uIsBVHDisplayed;
uniform bool uIsWireframeModeOn; 

const vec4 BVH_AABB_COLOR = vec4(0.5f, 0.f, 0.5f, 0.1f);
const vec4 BVH_AABB_LINE_COLOR = vec4(0.7f, 0.f, 0.7f, 0.1f);
const float WIREFRAME_LINE_WIDTH = 0.02f;
const float BVH_LINE_WIDTH = 0.05f;

layout (binding = 2, std430) readonly buffer uMaterialsSSBO {
    Material uMaterials[];
};

layout (binding = 3, std430) readonly buffer uTrianglesSSBO {
    Triangle uTriangles[];
};

layout (binding = 4, std430) readonly buffer uModelsSSBO {
    Model uModels[];
};

layout (binding = 5, std430) readonly buffer uBVH_SSBO {
    BVH_Node uBVH_Nodes[];
};


// code
Ray getRay(vec2 pos){ // pos between 0 and 1
    Ray ray;
    ray._Origin = uCamera._Eye;
    vec3 posViewSpace =  vec3(pos - 0.5f, 1.f) * vec3(uCamera._PlaneWidth, uCamera._PlaneHeight, uCamera._PlaneNear);
    vec4 posWorldSpace = uCamera._InvView * vec4(posViewSpace, 1.f);
    ray._Direction = normalize(posWorldSpace - ray._Origin);
    ray._Direction.w = 0.;
    return ray;
}

Hit rayTriangleIntersection(Ray ray, uint triangleIndex){
    Hit hit;

    Triangle triangle = uTriangles[triangleIndex];

    vec3 p0 = (uModels[triangle._ModelId]._ModelMatrix * triangle._P0).xyz;
    vec3 p1 = (uModels[triangle._ModelId]._ModelMatrix * triangle._P1).xyz;
    vec3 p2 = (uModels[triangle._ModelId]._ModelMatrix * triangle._P2).xyz;

    vec3 triEdge0 = p1 - p0;
    vec3 triEdge1 = p2 - p0;
    vec3 triNormale = normalize(cross(triEdge1, triEdge0));

    vec3 q = cross(ray._Direction.xyz, triEdge1);
    float a = dot(triEdge0, q);
    float epsilon = 1e-4;

    if(dot(triNormale, ray._Direction.xyz) >= 0 || abs(a) < epsilon){
        hit._DidHit = 0;
        return hit;
    }

    vec4 s = (ray._Origin - vec4(p0, 1.f)) / a;
    vec3 r = cross(s.xyz, triEdge0);

    hit._Coords.x = dot(s.xyz, q);
    hit._Coords.y = dot(r, ray._Direction.xyz);
    hit._Coords.z = 1 - hit._Coords.x - hit._Coords.y;

    if(hit._Coords.x < 0 || hit._Coords.y < 0 || hit._Coords.z < 0){
        hit._DidHit = 0;
        return hit;
    }
    
    float t = dot(triEdge1, r);
    if(t < 0){
        hit._DidHit = 0;
        return hit;
    }

    hit._DidHit = 1;
    hit._Coords.w = t;
    hit._TriangleId = triangleIndex;

    return hit;
}

void getAllHits(Ray ray, uint nbTriangles, inout Hit closestHit){
    for(uint i=0; i<nbTriangles; i++){
        Hit curHit = rayTriangleIntersection(ray, i);
        if(curHit._DidHit == 0) continue;
        if(closestHit._DidHit == 0 || curHit._Coords.w < closestHit._Coords.w){
            closestHit = curHit;
        }
    }
}

void getColor(Hit hit, vec4 bvhColor, inout vec4 color){
    // bvh color
    if(uIsBVHDisplayed){
        color = bvhColor;
    }

    if(hit._DidHit == 0) return;
    Triangle hitTriangle = uTriangles[hit._TriangleId];
    color += uMaterials[uModels[hitTriangle._ModelId]._MaterialId]._Color;

    // wireframe color
    if(uIsWireframeModeOn){
        // check distance to edges
        float threshold = WIREFRAME_LINE_WIDTH;
        if(hit._Coords.x < threshold
            || hit._Coords.y < threshold
            || hit._Coords.z < threshold){
            color = vec4(0.f, 0.f, 0.f, 1.f);
        }
    }
}


uint intersectBVH(Ray ray, BVH_Node node){
    float tMin = 0.f;
    float tMax = -1.f;

    // Check intersection with X-slabs
    float inverseRayDirX = 1.0f / ray._Direction.x;
    float tx1 = (node._BoundingBox._Min.x - ray._Origin.x) * inverseRayDirX;
    float tx2 = (node._BoundingBox._Max.x - ray._Origin.x) * inverseRayDirX;

    tMin = min(tx1, tx2);
    tMax = max(tx1, tx2);

    // Check for early exit
    if(tMax < 0.f || tMin > tMax) {
        return 0;
    }

    // Check intersection with Y-slabs
    float inverseRayDirY = 1.0f / ray._Direction.y;
    float ty1 = (node._BoundingBox._Min.y - ray._Origin.y) * inverseRayDirY;
    float ty2 = (node._BoundingBox._Max.y - ray._Origin.y) * inverseRayDirY;

    tMin = max(tMin, min(ty1, ty2));
    tMax = min(tMax, max(ty1, ty2));

    // Check for early exit
    if(tMax < 0.f || tMin > tMax) {
        return 0;
    }

    // Check intersection with Z-slabs
    float inverseRayDirZ = 1.0f / ray._Direction.z;
    float tz1 = (node._BoundingBox._Min.z - ray._Origin.z) * inverseRayDirZ;
    float tz2 = (node._BoundingBox._Max.z - ray._Origin.z) * inverseRayDirZ;

    tMin = max(tMin, min(tz1, tz2));
    tMax = min(tMax, max(tz1, tz2));

    // Check for intersection
    if(tMax >= 0.f && tMin <= tMax){
        // check if border
        float threshold = BVH_LINE_WIDTH / (uDepthDisplayBVH + 1.f);
        vec3 enterPoint = ray._Origin.xyz + ray._Direction.xyz * tMin;
        bool closeToX = (abs(enterPoint.x - node._BoundingBox._Min.x) < threshold) 
            || (abs(enterPoint.x - node._BoundingBox._Max.x) < threshold);
        bool closeToY = (abs(enterPoint.y - node._BoundingBox._Min.y) < threshold) 
            || (abs(enterPoint.y - node._BoundingBox._Max.y) < threshold);
        bool closeToZ = (abs(enterPoint.z - node._BoundingBox._Min.z) < threshold) 
            || (abs(enterPoint.z - node._BoundingBox._Max.z) < threshold);
        if((closeToX && closeToY) || (closeToX && closeToZ) || (closeToY && closeToZ)){
            return 2;
        }
        return 1;
    }
    return 0;
}

uint isLeafBVH(BVH_Node node){
    return 
        node._LeftChild == 0
        && node._RightChild == 0
        ? 1 : 0;
}

Hit getClosestHitBVH(Ray ray, uint rootBvh, inout vec4 bvhColor){
    Hit closestHit;
    closestHit._DidHit = 0;

    // Create a stack for the node indices
    const uint STACK_SIZE = 1024;
    uint stack[STACK_SIZE];
    uint depthStack[STACK_SIZE];
    int stackIndex = 0;
    // Push the root node onto the stack
    stack[stackIndex] = rootBvh;
    depthStack[stackIndex] = 0;
    stackIndex++;
    // Iterate while the stack is not empty
    while (stackIndex > 0) {
        // Pop the current node
        stackIndex--;
        uint currentNodeIndex = stack[stackIndex];
        uint currentDepth = depthStack[stackIndex];
        BVH_Node curNode = uBVH_Nodes[currentNodeIndex];
        // Check if the ray intersects the current BVH node's bounding box
        uint intersectionBVH = intersectBVH(ray, curNode);
        if(intersectionBVH != 0) {
            if(currentDepth == uDepthDisplayBVH){
                if(intersectionBVH == 2){
                    bvhColor = BVH_AABB_LINE_COLOR;
                } else {
                    bvhColor = BVH_AABB_COLOR;
                }
            }
            // Check if the current node is a leaf
            if(isLeafBVH(curNode) == 1) {
                Hit hit = rayTriangleIntersection(ray, curNode._TriangleId);
                if(closestHit._DidHit == 0 || hit._Coords.w < closestHit._Coords.w){
                    closestHit = hit;
                }
            } else {
                // Push the children onto the stack
                stack[stackIndex] = curNode._LeftChild;
                depthStack[stackIndex] = currentDepth+1;
                stackIndex++;
                stack[stackIndex] = curNode._RightChild;
                depthStack[stackIndex] = currentDepth+1;
                stackIndex++;
            }
        }
    }

    return closestHit;
}


// main
void main() {
    vec4 value = vec4(0.f, 0.f, 0.f, 1.f);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 pixelPos = vec2(0.f);
    pixelPos.x = float(texelCoord.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    pixelPos.y = float(texelCoord.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    Ray ray = getRay(pixelPos);

    // // no bvh
    // Hit closestHit;
    // closestHit._DidHit = 0;
    // getAllHits(ray, uNbTriangles, closestHit);

    // bvh
    uint rootBvh = 0;
    vec4 bvhColor = vec4(0.f, 0.f, 0.f, 0.f);
    Hit closestHit = getClosestHitBVH(ray, rootBvh, bvhColor);

    getColor(closestHit, bvhColor, value);

    // BVH_Node root = uBVH_Nodes[rootBvh];
    // uint nbClusters = 2*uNbTriangles-1;
    // value = vec4(
    //     root._LeftChild / (1.f*nbClusters), 
    //     root._RightChild / (1.f*nbClusters),
    //     root._TriangleId / (1.f*uNbTriangles),
    //     1.f
    // );

    imageStore(oImage, texelCoord, value);
}