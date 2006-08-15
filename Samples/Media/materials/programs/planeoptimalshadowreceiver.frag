/////////////////////////////////////////////////////////////////////////////////
//
// shadowreceiver.frag
//
// Hamilton Chong
// (c) 2006
//
/////////////////////////////////////////////////////////////////////////////////

// I N P U T   V A R I A B L E S ////////////////////////////////////////////////

// uniform constants
uniform sampler2D    uShadowMap;
uniform float        uSTexWidth;
uniform float        uSTexHeight;

// per fragment inputs
varying vec4   pShadowCoord;    // vertex position in shadow map coordinates
varying float  pDiffuse;        // diffuse shading value

// M A I N //////////////////////////////////////////////////////////////////////

void main(void)
{
    // compute the shadow coordinates for texture lookup
    // NOTE: texture_viewproj_matrix maps z into [0,1] range, not [-1,1], so
    //  have to make sure shadow caster stores depth values with same convention.
    vec4 scoord = pShadowCoord / pShadowCoord.w;

    // ------------------------------------------------------------------------------

    // Non-PCF code (comment out above section and uncomment the following two lines)

    float zvalue = texture2D(uShadowMap, scoord.xy).x;
    float visibility = (zvalue <= scoord.z) ? 0.0 : 1.0;
    visibility = ((abs(scoord.x - 0.5) <= 0.5) && (abs(scoord.y - 0.5) <= 0.5)) ? visibility : 1.0;
    
    // ------------------------------------------------------------------------------

    visibility *= pDiffuse;
    gl_FragColor = vec4(visibility, visibility, visibility, 0.0);
}