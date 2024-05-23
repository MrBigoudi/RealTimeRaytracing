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
    vec3 _Origin;
    vec3 _Direction;
};

struct Triangle {
    uint _Id;
    vec3 _P0;
    vec3 _P2;
    vec3 _P1;
};

struct Hit {
    uint _DidHit;
    vec3 _BarycentricCoords;
    float _DistToOrigin;
    uint _TriangleId;
};



// output
layout(rgba32f, binding = 0) uniform image2D oImage;

// input
layout(local_size_x = 10, local_size_y = 10, local_size_z = 1) in;
layout (location = 0) uniform float uTime;
layout (location = 1) uniform Camera uCamera;


// code

Ray getRay(vec2 pos){ // pos between 0 and 1
    Ray ray;
    ray._Origin = uCamera._Eye.xyz;
    vec3 posViewSpace =  vec3(pos - 0.5f, 1.f) * vec3(uCamera._PlaneWidth, uCamera._PlaneHeight, uCamera._PlaneNear);
    vec3 posWorldSpace = (uCamera._InvView * vec4(posViewSpace, 1.f)).xyz;
    ray._Direction = normalize(posWorldSpace - ray._Origin);
    return ray;
}

Hit rayTriangleIntersection(Ray ray, Triangle triangle){
    Hit hit;
    vec3 triEdge0 = triangle._P1 - triangle._P0;
    vec3 triEdge1 = triangle._P2 - triangle._P0;
    vec3 triNormale = normalize(cross(triEdge0, triEdge1));

    vec3 q = cross(ray._Direction, triEdge1);
    float a = dot(triEdge0, q);
    float epsilon = 1e-4;

    if(dot(triNormale, ray._Direction) >= 0 || abs(a) < epsilon){
        hit._DidHit = 0;
        return hit;
    }

    vec3 s = (ray._Origin - triangle._P0) / a;
    vec3 r = cross(s, triEdge0);

    hit._BarycentricCoords.x = dot(s, q);
    hit._BarycentricCoords.y = dot(r, ray._Direction);
    hit._BarycentricCoords.z = 1 - hit._BarycentricCoords.x - hit._BarycentricCoords.y;

    if(hit._BarycentricCoords.x < 0 || hit._BarycentricCoords.y < 0 || hit._BarycentricCoords.z < 0){
        hit._DidHit = 0;
        return hit;
    }
    
    float t = dot(triEdge1, r);
    if(t < 0){
        hit._DidHit = 0;
        return hit;
    }

    hit._DidHit = 1;
    hit._DistToOrigin = t;
    hit._TriangleId = triangle._Id;

    return hit;
}

// temporary
const Triangle SHARED_TRIANGLE = {
    0,
    vec3(-1, -1, 1),
    vec3(1, -1, 1),
    vec3(0, 1, 1),
};

void main() {
    vec4 value = vec4(0.f, 0.f, 0.f, 1.f);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

    vec2 pixelPos = vec2(0.f);
    pixelPos.x = float(texelCoord.x) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
    pixelPos.y = float(texelCoord.y)/(gl_NumWorkGroups.y * gl_WorkGroupSize.y);

    Ray ray = getRay(pixelPos);
    Hit hit = rayTriangleIntersection(ray, SHARED_TRIANGLE);

    value.xyz = hit._DidHit != 0 ? vec3(1.f) : value.xyz;

    imageStore(oImage, texelCoord, value);
}