/////////////////////////////////////////////////////////////////////////////////
//
// SimpleShaderFP.cg
//
// Hamilton Chong
// (c) 2006
//
//
/////////////////////////////////////////////////////////////////////////////////


sampler2D TextureMap : TEXUNIT0;

// Define outputs from vertex shader.
struct Vertex
{
  float4 position       : POSITION;     // fragment position in post projective space
  float4 texCoord    	: TEXCOORD0;    // fragment position in shadow map coordinates
  float  diffuse        : COLOR;    	// diffuse shading value
};

struct Fragment
{
    float4 color  : COLOR0;
};

Fragment main(Vertex In)
{
    Fragment Out;

    Out.color = tex2D(TextureMap, In.texCoord);
    // The ogre head model does not seem to have an outward pointing normal...
    //Out.color.xyz *= In.diffuse;

    return Out;
}