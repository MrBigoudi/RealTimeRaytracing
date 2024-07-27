module raytracer;

__include "structures.slang";

static const int THREAD_GROUP_SIZE_X = 16;
static const int THREAD_GROUP_SIZE_Y = 16;
static const int THREAD_GROUP_SIZE_Z = 1;

// output
RWTexture2D oImage : register(u0);

// uniforms
uniform float uTime;
uniform Camera uCamera;
uniform uint uNbMaterials;
uniform uint uNbTriangles;
uniform uint uNbModels;
uniform int uDepthDisplayBVH;
uniform bool uIsBVHDisplayed;
uniform bool uIsWireframeModeOn; 

[shader("compute")]
[numthreads(THREAD_GROUP_SIZE_X, THREAD_GROUP_SIZE_Y, THREAD_GROUP_SIZE_Z)]
void computeMain(uint3 threadId : SV_DispatchThreadID){
    uint2 texelCoord = threadId.xy;
	uint2 size = uint2(0, 0); // initialize to arbitrary value
    oImage.GetDimensions(size.x, size.y); // get size from bound image

    if(texelCoord.x < size.x && texelCoord.y < size.y)
    {
        Material mat = Material(float4(0.0, 0.0, 0.0, 1.0));

        if(texelCoord.x % THREAD_GROUP_SIZE_X != 0 && texelCoord.y % THREAD_GROUP_SIZE_Y != 0){
            mat._Color.r = float(texelCoord.x) / float(size.x);
            mat._Color.g = float(texelCoord.y) / float(size.y);
        }
    
        oImage[texelCoord.xy] = mat._Color;
        // oImage[texelCoord.xy] = Material()._Color;
    }
}