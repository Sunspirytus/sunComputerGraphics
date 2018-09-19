uniform samplerCube Skybox_Tex;

uniform lowp vec4 FogColor;
uniform mediump float FogHeightRatio;
uniform int bDrawFog;

varying mediump vec3 EyeDir;
varying mediump float VertexHeight;

void main()
{
	
	// Mix the object's colour with the fogging colour based on fragment's depth
	lowp vec3 SkyboxColor = textureCube(Skybox_Tex, EyeDir).rgb;
	
	if(bDrawFog == 0)
	{
		gl_FragColor = vec4(SkyboxColor, 1.0);
		return;
	}
	
	// Test depth
	lowp float fFogBlend = 1.0 - clamp(VertexHeight * FogHeightRatio, 0.0, 1.0);
	lowp vec3 vFragColor = mix(SkyboxColor.rgb, FogColor.rgb, fFogBlend);
	
	gl_FragColor = vec4(vFragColor.rgb, 1.0);
}