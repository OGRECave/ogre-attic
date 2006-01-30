sampler Sum: register(s0);
float4 main(float2 texCoord: TEXCOORD0) : COLOR {
   return tex2D(Sum, texCoord);
}

