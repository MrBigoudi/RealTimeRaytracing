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
    uint _Id;
    uint _MaterialId;
};

struct Material {
    vec4 _Color;
    uint _Id;
};

struct Hit {
    vec4 _Coords; // (b0, b1, b2, t)
    uint _DidHit;
    uint _TriangleId;
};


// constants
const uint MAX_NB_TRIANGLES = 2;//2 << 15;
const uint MAX_NB_MATERIALS = 2; //2 << 10;

// output
layout(rgba32f, binding = 0) uniform image2D oImage;

// input
layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

uniform float uTime;
uniform Camera uCamera;
uniform int uNbMaterials;
uniform int uNbTriangles;

layout (binding = 0, std140) uniform uGeometryData {
    Triangle uTriangles[MAX_NB_TRIANGLES];
    Material uMaterials[MAX_NB_MATERIALS];
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

Hit rayTriangleIntersection(Ray ray, Triangle triangle){
    Hit hit;
    vec3 triEdge0 = triangle._P1.xyz - triangle._P0.xyz;
    vec3 triEdge1 = triangle._P2.xyz - triangle._P0.xyz;
    vec3 triNormale = normalize(cross(triEdge0, triEdge1));

    vec3 q = cross(ray._Direction.xyz, triEdge1);
    float a = dot(triEdge0, q);
    float epsilon = 1e-4;

    if(dot(triNormale, ray._Direction.xyz) >= 0 || abs(a) < epsilon){
        hit._DidHit = 0;
        return hit;
    }

    vec4 s = (ray._Origin - triangle._P0) / a;
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
    hit._TriangleId = triangle._Id;

    return hit;
}

void getAllHits(Ray ray, uint nbTriangles, Triangle[MAX_NB_TRIANGLES] triangles, inout Hit closestHit){
    for(uint i=0; i<nbTriangles; i++){
        Triangle curTriangle = triangles[i];
        Hit curHit = rayTriangleIntersection(ray, curTriangle);
        if(curHit._DidHit == 0) continue;
        if(closestHit._DidHit == 0 || curHit._Coords.w < closestHit._Coords.w){
            closestHit = curHit;
        }
    }
}

void getColor(Material[MAX_NB_MATERIALS] materials, Triangle[MAX_NB_TRIANGLES] triangles, Hit hit, inout vec4 color){
    if(hit._DidHit == 0) return;
    color = materials[triangles[hit._TriangleId]._MaterialId]._Color;
}



// main
void main() {
    vec4 value = vec4(0.f, 0.f, 0.f, 1.f);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 pixelPos = vec2(0.f);
    pixelPos.x = float(texelCoord.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    pixelPos.y = float(texelCoord.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    Ray ray = getRay(pixelPos);

    Hit closestHit;
    closestHit._DidHit = 0;
    getAllHits(ray, uNbTriangles, uTriangles, closestHit);

    getColor(uMaterials, uTriangles, closestHit, value);

    imageStore(oImage, texelCoord, value);
}