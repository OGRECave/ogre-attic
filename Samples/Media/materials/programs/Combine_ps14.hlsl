float blur;

sampler RT;
sampler Sum;

float4 main(float2 texCoord: TEXCOORD0) : COLOR 
{
   float4 render = tex2D(RT, texCoord);
   float4 sum = tex2D(Sum, texCoord);

   return lerp(render, sum, blur);
}



