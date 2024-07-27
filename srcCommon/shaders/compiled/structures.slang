implementing raytracer;

struct Camera {
    float4x4 _View;
    float4x4 _Proj;
    float4x4 _InvView;
    float4x4 _InvProj;
    float4 _Eye;
    float _PlaneWidth;
    float _PlaneHeight;
    float _PlaneNear;
};

struct Ray {
    float4 _Origin;
    float4 _Direction;
};

struct Triangle {
    float4 _P0;
    float4 _P2;
    float4 _P1;
    uint _ModelId;
};

struct Material {
    float4 _Color;
    __init(){
        _Color = float4(0.1f, 0.2f, 0.1f, 1.f);
    }
    __init(float4 color){
        _Color = color;
    }
};

struct Model {
    float4x4 _ModelMatrix;
    uint _MaterialId;
};

struct Hit {
    float4 _Coords; // (b0, b1, b2, t)
    uint _DidHit;
    uint _TriangleId;
};

struct AABB {
    float3 _Min;
    float3 _Max;
};

struct BVH_Node {
    AABB _BoundingBox;
    uint _TriangleId;
    uint _LeftChild;
    uint _RightChild;
};