/////////////////////////////////////////////////////////////////////////////////
//
// shadowreceiverfp.cg
//
// Hamilton Chong
// (c) 2006
//
// This is an example fragment shader for shadow receiver objects.  
//
/////////////////////////////////////////////////////////////////////////////////


sampler2D ShadowMap : TEXUNIT0;

// Define outputs from vertex shader.
struct Vertex
{
  float4 position       : POSITION;     // fragment position in post projective space
  float4 shadowCoord    : TEXCOORD0;    // fragment position in shadow map coordinates
  float  diffuse        : TEXCOORD1;    	// diffuse shading value
};

struct Fragment
{
    float4 color  : COLOR0;
};

Fragment main(Vertex        In, 
              uniform float uSTexWidth,
              uniform float uSTexHeight)
{
    Fragment Out;

    // compute the shadow coordinates for texture lookup
    // NOTE: texture_viewproj_matrix maps z into [0,1] range, not [-1,1], so
    //  have to make sure shadow caster stores depth values with same convention.
    float4 scoord = In.shadowCoord / In.shadowCoord.w;


    // -- "Percentage Closest Filtering" ----------------------------------------- 

    // One could use scoord.xy to look up the shadow map for depth testing, but
    // we'll be implementing a simple "percentage closest filtering" algorithm instead.
    // This mimics the behavior of turning on bilinear filtering on NVIDIA hardware
    // when also performing shadow comparisons.  This causes bilinear filtering of
    // depth tests.  Note that this is NOT the same as bilinear filtering the depth
    // values and then doing the depth comparison.  The two operations are not 
    // commutative.  PCF is explicitly about filtering the test values since
    // testing filtered z values is often meaningless.  

    // Real percentage closest filtering should sample from the entire footprint
    // on the shadow map, not just seek the closest four sample points.  Such 
    // an improvement is for future work.

    
    // NOTE: Assuming DX convention of having integers mark sample corners
    //  OpenGL convention for texture lookups is having integers in centers.
    float2 tcoord;
    tcoord.x = (scoord.x * uSTexWidth) - 0.5;
    tcoord.y = (scoord.y * uSTexHeight) - 0.5;
    float x0 = floor(tcoord.x) + 0.5;
    float x1 = ceil(tcoord.x) + 0.5;
    float fracx = frac(tcoord.x);
    float y0 = floor(tcoord.y) + 0.5;
    float y1 = ceil(tcoord.y) + 0.5;
    float fracy = frac(tcoord.y);
    
    // sample coordinates in [0,1]^2 domain
    float2 t00, t01, t10, t11;
    float invWidth  = 1.0 / uSTexWidth;
    float invHeight = 1.0 / uSTexHeight;
    t00 = float2((x0) * invWidth, (y0) * invHeight);
    t10 = float2((x1) * invWidth, (y0) * invHeight);
    t01 = float2((x0) * invWidth, (y1) * invHeight);
    t11 = float2((x1) * invWidth, (y1) * invHeight);
    
    // grab the samples
    float z00 = tex2D(ShadowMap, t00).x;
    float viz00 = (z00 <= scoord.z) ? 0.0 : 1.0;
    float z01 = tex2D(ShadowMap, t01).x;
    float viz01 = (z01 <= scoord.z) ? 0.0 : 1.0;
    float z10 = tex2D(ShadowMap, t10).x;
    float viz10 = (z10 <= scoord.z) ? 0.0 : 1.0;
    float z11 = tex2D(ShadowMap, t11).x;
    float viz11 = (z11 <= scoord.z) ? 0.0 : 1.0;

    // determine that all geometry outside the shadow test frustum is lit
    viz00 = (all(abs(t00-0.5)<=0.5)) ? viz00 : 1.0;
    viz01 = (all(abs(t01-0.5)<=0.5)) ? viz01 : 1.0;
    viz10 = (all(abs(t10-0.5)<=0.5)) ? viz10 : 1.0; 
    viz11 = (all(abs(t11-0.5)<=0.5)) ? viz11 : 1.0;

    // bilinear filter test results
    float v0 = (1.0 - fracx) * viz00 + fracx * viz10;
    float v1 = (1.0 - fracx) * viz01 + fracx * viz11;
    float visibility = (1.0 - fracy) * v0 + fracy * v1;

    // ------------------------------------------------------------------------------

    // Non-PCF code (comment out above section and uncomment the following three lines)

    //float zvalue = tex2D(ShadowMap, scoord.xy).x;
    //float visibility = (zvalue <= scoord.z) ? 0.0 : 1.0;
    //visibility = (all(abs(scoord.xy-0.5)<=0.5)) ? visibility : 1.0;
    
    // ------------------------------------------------------------------------------

    visibility *= In.diffuse;
    Out.color = float4(visibility, visibility, visibility, 0.0);
    return Out;
}