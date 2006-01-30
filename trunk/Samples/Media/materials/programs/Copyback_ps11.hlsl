sampler Temp;

float4 main(float2 texCoord: TEXCOORD0) : COLOR 
{
   return tex2D(Temp, texCoord);
}

