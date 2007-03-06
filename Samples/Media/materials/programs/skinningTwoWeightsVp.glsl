// Example GLSL program for skinning with two bone weights per vertex

attribute vec4 blendIndices;
attribute vec4 blendWeights;

// 3x4 matrix, passed as vec4's for compatibility with GL 2.0
// GL 2.0 supports 3x4 matrices
// Support 24 bones ie 24*3, but use 72 since our parser can pick that out for sizing
uniform vec4 worldMatrix3x4Array[72];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos[2];
uniform vec4 lightDiffuseColour[2];

void main()
{
	vec3 blendPos = vec3(0,0,0);
	vec3 blendNorm = vec3(0,0,0);
	
	vec3 tmpPos = vec3(0,0,0);
	vec3 tmpNorm = vec3(0,0,0);


	for (int bone = 0; bone < 2; ++bone)
	{
		// perform matrix multiplication manually since no 3x4 matrices
        // ATI GLSL compiler can't handle indexing an array within an array so calculate the inner index first
	    int idx = int(blendIndices[bone]) * 3;
	    vec4 blendMatrixRow;
//  ATI GLSL compiler can't handle unrolling the loop 
//		for (int row = 0; row < 3; ++row)
//		{
//			blendMatrixRow = worldMatrixArray[idx + row];
//			tmpPos[row] = dot(blendMatrixRow, gl_Vertex);
//
//			tmpNorm[row] = dot(blendMatrixRow.xyz, gl_Normal);
//		}
        blendMatrixRow = worldMatrix3x4Array[idx];
		tmpPos[0] = dot(blendMatrixRow, gl_Vertex);
		tmpNorm[0] = dot(blendMatrixRow.xyz, gl_Normal);
		
        blendMatrixRow = worldMatrix3x4Array[idx + 1];
		tmpPos[1] = dot(blendMatrixRow, gl_Vertex);
		tmpNorm[1] = dot(blendMatrixRow.xyz, gl_Normal);
		
        blendMatrixRow = worldMatrix3x4Array[idx + 2];
		tmpPos[2] = dot(blendMatrixRow, gl_Vertex);
		tmpNorm[2] = dot(blendMatrixRow.xyz, gl_Normal);
		
		// now weight this into final 
	    float weight = blendWeights[bone];
		blendPos += tmpPos * weight;
		blendNorm += tmpNorm * weight;
	}

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1);

	// simple lighting model
	vec3 lightDir0 = normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	vec3 lightDir1 = normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));
		
	gl_FrontSecondaryColor = vec4(0,0,0,0);
	gl_FrontColor = vec4(0.5, 0.5, 0.5, 1.0) 
		+ clamp(dot(lightDir0, blendNorm), 0.0, 1.0) * lightDiffuseColour[0]
		+ clamp(dot(lightDir1, blendNorm), 0.0, 1.0) * lightDiffuseColour[1];

	gl_TexCoord[0] = gl_MultiTexCoord0;
	
}
