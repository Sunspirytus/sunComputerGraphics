#version 100
uniform sampler2D diffuseMap;
uniform sampler2D LargeWaves_NormalTex;
uniform sampler2D SmallWaves_NormalTex;
uniform sampler2D SeaFoam1_Tex;
uniform samplerCube	Skybox_Tex;
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

//These parameters are used in both vertex and fragment Shader, !!!must keep them same in VF Shader.!!!
const highp float WaterScale = 1.0;
const highp float WaterSpeed = 0.2;
const highp float LargeWaveScale = 1000.0;
const highp float LargeWaveSpeed = 2.0;
const highp int chaosWay = 4;

//These parameters are used only in fragment Shader.
const highp float SpecularPower = 128.0;
const highp float LargeWaveAmplify = 0.9;
const highp float SmallWaveScale = 500.0;
const highp float SmallWaveSpeed = 1.0;
const highp float SmallWaveAmplify = 0.2;
const highp float SeaFoamScale = 500.0;
const highp float SeaFoamSpeed = 1.0;
const highp float SeaFoamDistortion = 0.0;
const highp float SeaFoamHeightPower = 2.0;
const highp float SeaFoamHeightMultiply = 3.0;
const highp float ReflectionAmount = 0.5;
highp vec4 OceanWaterColor = vec4(0.133, 0.145, 0.34, 1.0); //Need Calculate transparent, not be const
highp vec4 OceanWaterFresnelColor = vec4(0.248, 0.25, 0.395, 1.0);


const highp vec3 diffuseColor = vec3(1.0, 0.0, 0.0);
const highp vec3 lightColor = vec3(1.0, 1.0, 1.0);

mediump float lerp(mediump float a, mediump float b, mediump float w) {
  return a + w*(b-a);
}

highp vec3 MotionNWayChaos(highp vec2 chaosUV, highp float speed, highp sampler2D tex, highp int way){
	highp vec3 color = vec3(0.0, 0.0, 0.0);
	
	highp vec2 uv0 = chaosUV + vec2(0.1, 0.1) * speed * _Time;
	color += texture2D(tex, uv0).rgb;
	if(way == 1) return color;
	
	highp vec2 uv1 =  vec2(0.42, 0.26) + chaosUV + vec2(-0.1, -0.1) * speed * _Time;
	color += texture2D(tex, uv1).rgb;
	if(way == 2) return color / 2.0;
	
	highp vec2 uv2 = vec2(0.87, 0.15) + chaosUV + vec2(-0.1, 0.1) * speed * _Time;
	color += texture2D(tex, uv2).rgb;
	if(way == 3) return color / 3.0;
	
	highp vec2 uv3 = vec2(0.65, 0.75) + chaosUV + vec2(0.1, -0.1) * speed * _Time;
	color += texture2D(tex, uv3).rgb;
	if(way ==4) return color / 4.0;
}

highp float FresnelReflect(highp vec3 normal, highp vec3 viewDir, highp float power){
	return pow(1.0 - clamp(dot(normal, viewDir), 0.0, 1.0), power);	
}


void main()
{
	////////////////////////////////
	//Calculate Large Wave normal.//
	////////////////////////////////
	highp vec2 chaosUV_Large = (Vertex_World / WaterScale / LargeWaveScale).xz; //Care dir
	//Never forget normalTangentTex * 2.0 - 1.0;
	highp vec3 LargeWaveNormalAfterChaos_Tangent = MotionNWayChaos(chaosUV_Large, WaterSpeed * LargeWaveSpeed, LargeWaves_NormalTex, chaosWay) * 2.0 - 1.0;
	
	highp vec3 LargeWaveNormalAfterChaos_World = TangentToWorldMatrix * LargeWaveNormalAfterChaos_Tangent;
	highp vec3 LargeWaveNormal_World = LargeWaveNormalAfterChaos_World;
	LargeWaveNormal_World *= mix(vec3(1.0, 3.0, 1.0), vec3(1.0, 0.1, 1.0), LargeWaveAmplify);//Care dir

	////////////////////////////////
	//Calculate Small Wave normal.//
	////////////////////////////////
	highp vec2 chaosUV_Small = (Vertex_World / WaterScale / SmallWaveScale).xz; //Care dir
	//Never forget normalTangentTex * 2.0 - 1.0;
	highp vec3 SmallWaveNormalAfterChaos_Tangent = MotionNWayChaos(chaosUV_Small, WaterSpeed * SmallWaveSpeed, SmallWaves_NormalTex, chaosWay) * 2.0 - 1.0;
	//Same as Large Wave
	highp vec3 SmallWaveNormal_World = TangentToWorldMatrix * SmallWaveNormalAfterChaos_Tangent;
	SmallWaveNormal_World *= mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 0.2, 1.0), SmallWaveAmplify); //Care dir
	
	////////////////////////////////
	///////////Normal with /////////
	////Large Wave and Small Wave///
	//////// in world spase/////////
	////////////////////////////////
	highp vec3 Normal_World = normalize(LargeWaveNormal_World + SmallWaveNormal_World);	
	
	////////////////////////////////
	//////Calculate Ocean Water/////
	////////without Sea Foam////////
	////////////////////////////////
	OceanWaterColor *= OceanWaterColor.a;
	OceanWaterFresnelColor *= OceanWaterFresnelColor.a;

	highp float fresnelRefract = clamp(1.0 - FresnelReflect(Normal_World, ViewDir_WorldSpace, 5.0), 0.0, 1.0);
	/*issue about transparent*/
	highp vec3 OceanColorWithoutSeaFoam = mix(OceanWaterColor.rgb, OceanWaterFresnelColor.rgb, fresnelRefract); //Care transparent!!!
	
	////////////////////////////////
	//////Calculate Ocean Water/////
	/////////with Sea Foam//////////
	////////////////////////////////
	highp vec2 chaosUV_SeaFoam = (Vertex_World / WaterScale / SeaFoamScale).xz; //Care dir
	chaosUV_SeaFoam += (1.0 - LargeWaveNormalAfterChaos_World.xz) * SeaFoamDistortion; //Care dir
	highp vec3 SeaFoamColorAfterChaos = MotionNWayChaos(chaosUV_SeaFoam, WaterSpeed * SeaFoamSpeed, SeaFoam1_Tex, chaosWay);
	/*issue about "g"*/
	highp vec3 OceanColorWithSeaFoam = mix(OceanColorWithoutSeaFoam, SeaFoamColorAfterChaos, SeaFoamColorAfterChaos.g);
	highp float VertexOffsetSeaFoam = clamp(SeaFoamHeightMultiply * pow(VertexOffsetAfterChaos, SeaFoamHeightPower), 0.0, 1.0);
	OceanColorWithSeaFoam = mix(OceanColorWithoutSeaFoam, OceanColorWithSeaFoam, VertexOffsetSeaFoam);
	
	////////////////////////////////
	////Sea Foam with Reflection////
	////////////////////////////////
	highp float SeaFoamReflectionRatio = fresnelRefract * clamp(1.0 - (SeaFoamColorAfterChaos.g * VertexOffsetSeaFoam) * 3.0, 0.0, 1.0);
	highp vec3 SeaFoamWithReflection = ReflectionAmount * SeaFoamReflectionRatio * textureCube(Skybox_Tex, reflect(ViewDir_WorldSpace, Normal_World)).rgb;
	SeaFoamWithReflection += OceanColorWithSeaFoam;
	
	
	
	
	
	highp vec2 uv = texcoord;
	
	highp float NdotL = dot(Normal_World, LightDir_WorldSpace);
	highp vec3 HalfView_WorldSpace = normalize(Normal_World + ViewDir_WorldSpace);
	
	highp float diffuseTerm = NdotL;
	highp float specularTerm = pow(dot(Normal_World, HalfView_WorldSpace), SpecularPower);
	
	highp vec3 diffuse = lightColor * 1.2 * SeaFoamWithReflection * diffuseTerm;
	highp vec3 specular = lightColor * specularTerm;
	
	highp vec3 color = diffuse + specular;
	
	highp float fogBlend = pow(clamp(EyeToVertexDis * FogDepthRatio, 0.0, 1.0), 3.0);
	color = mix(color, FogColor.rgb, fogBlend);
		
	gl_FragColor = vec4(color, 1.0);
}