// Example GLSL program for skinning with two bone weights per vertex

attribute vec4 blendIndices;
attribute vec4 blendWeights;

// 3x4 matrix, passed as vec4's for compatibility with GL 2.0
// GL 2.0 supports 3x4 matrices
// Support 24 bones ie 24*3, but use 72 since our parser can pick that out for sizing
uniform vec4 worldMatrixArray[72];
uniform mat4 viewProjMatrix;
uniform vec4 lightPos[2];
uniform vec3 lightDiffuseColour[2];
uniform vec3 ambient;

void main()
{
	vec3 blendPos = vec3(0,0,0);
	vec3 blendNorm = vec3(0,0,0);
	
	vec3 tmpPos = vec3(0,0,0);
	vec3 tmpNorm = vec3(0,0,0);


	for (int bone = 0; bone < 2; ++bone)
	{
		// perform matrix multiplication manually since no 3x4 matrices
		for (int row = 0; row < 3; ++row)
		{
			vec4 blendMatrixRow = worldMatrixArray[int(blendIndices[bone]) * 3 + row];
			tmpPos[row] = dot(blendMatrixRow, gl_Vertex);

			tmpNorm[row] = dot(blendMatrixRow.xyz, gl_Normal);
			
		}
		// now weight this into final 
		blendPos += tmpPos * blendWeights[bone];
		blendNorm += tmpNorm * blendWeights[bone];
	}

	// apply view / projection to position
	gl_Position = viewProjMatrix * vec4(blendPos, 1);

	// simple lighting model
	vec3 lightDir0 = normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	vec3 lightDir1 = normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));
	
	gl_FrontColor = vec4(
		ambient 
		+ clamp(dot(lightDir0, blendNorm), 0.0, 1.0) * lightDiffuseColour[0]
		+ clamp(dot(lightDir1, blendNorm), 0.0, 1.0) * lightDiffuseColour[1], 1);
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
}
