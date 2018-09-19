#version 100
uniform sampler2D SmallWaves_NormalTex;
uniform sampler2D ReflectionTex;
uniform sampler2D RefractionTex;
uniform sampler2D Skybox_Tex;
uniform highp float _Time;
uniform highp vec4 FogColor;
uniform highp float FogDepthRatio;

varying mediump vec2 texcoord;
varying mediump vec3 LightDir_WorldSpace;
varying mediump vec3 ViewDir_WorldSpace;
varying mediump vec3 Vertex_World;
varying mediump float VertexOffsetAfterChaos;

varying mediump mat3 TangentToWorldMatrix;
varying mediump float EyeToVertexDis;
varying mediump vec3 colorOut;

varying mediump vec4 NormalCoords;
varying mediump vec4 NormalAfterDistortion_World;
varying mediump vec4 ReflectionUV;
varying mediump vec4 RefractionUV;

const highp float VertexDisplacement = 1.0;
const highp float DistorationStrength = 4.5;
const highp float Shininess = 128.0;
const highp float FresnelScale = 0.55;
const highp float FresnelBias = 0.11;
const highp float FresnelPower = 2.8;
const highp vec4 BaseColor = vec4(0.172, 0.223, 0.247, 0.5);
const highp vec4 ReflectionColor = vec4(0.475, 0.61, 0.66, 0.48);


const highp vec3 diffuseColor = vec3(1.0, 0.0, 0.0);
const highp vec3 LightColor = vec3(0.431, 0.373, 0.333);

highp float FresnelReflect(highp vec3 normal, highp vec3 viewDir, highp float power){
	return pow(1.0 - clamp(dot(normal, viewDir), 0.0, 1.0), power);	
}

highp vec3 PerPixelNormal(sampler2D normalTex, highp vec4 coords, highp vec3 normalAfterDistortion_World, highp float bumpStrength)
{
	highp vec3 normalMap = ((texture2D(normalTex, coords.xy).rgb * 2.0 - 1.0) + (texture2D(normalTex, coords.zw).rgb * 2.0 - 1.0)) * 0.5;
	highp vec3 normal_World = normalAfterDistortion_World + normalMap.xxy * bumpStrength * vec3(1.0, 0.0, 1.0); //Care dir
	return normalize(normal_World);
}

highp float Fresnel(highp vec3 viewDir, highp vec3 normal, highp float bias, highp float power)
{
	highp float facing = clamp(1.0 - max(dot(viewDir, normal), 0.0), 0.0, 1.0);
	highp float refl2Refr = max(0.0, bias + (1.0 - bias) * pow(facing, power));
	return refl2Refr;
}

void main()
{

	highp vec3 normal_World = PerPixelNormal(SmallWaves_NormalTex, NormalCoords, NormalAfterDistortion_World.xyz, VertexDisplacement);
	highp vec2 distoratOffset = vec2(normal_World.xz * DistorationStrength * 10.0);
	highp vec2 reflectionUV = ReflectionUV.xy + distoratOffset / ReflectionUV.w;
	highp vec2 refractionUV = RefractionUV.xy + distoratOffset / RefractionUV.w;
	
	highp vec3 reflectionColorTex = texture2D(ReflectionTex, reflectionUV).rgb;
	highp vec3 refractionColorTex = texture2D(RefractionTex, refractionUV).rgb;
	
	highp vec3 halfView_World = normalize(LightDir_WorldSpace + ViewDir_WorldSpace);
	highp float specularTerm = pow(max(0.0, dot(halfView_World, normal_World)), Shininess);
	 
	normal_World.xz *= FresnelScale;
	highp float refl2Refr = Fresnel(ViewDir_WorldSpace, normal_World, FresnelBias, FresnelPower);
	highp vec4 ColorBase = BaseColor;
	highp vec3 ColorReflect = mix(reflectionColorTex.rgb, ReflectionColor.rgb, ReflectionColor.a);
	highp vec3 ColorRefract = mix(refractionColorTex.rgb, BaseColor.rgb, BaseColor.a);
	highp vec3 color = mix(ColorRefract, ColorReflect, refl2Refr);
	color += specularTerm * LightColor;	
	
	
	highp float fogBlend = pow(clamp(EyeToVertexDis * FogDepthRatio, 0.0, 1.0), 3.0);
	color = mix(color, FogColor.rgb, fogBlend);
	
	gl_FragColor = vec4(color, 1.0);
}