#version 100
uniform highp mat4 MVPMatrix;
uniform highp mat4 MMatrix_IT;
uniform highp mat4 MMatrix;
uniform mediump vec3 LightDirModel;
uniform highp vec3 EyePosModel;
uniform highp float _Time;
uniform sampler2D LargeWaves_HightTex;

attribute highp vec3 inVertex;
attribute mediump vec3 inNormal;
attribute mediump vec3 inTangent;
attribute mediump vec3 inBiNormal;
attribute highp vec2 inTexCoords;

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

//These parameters are only used in vertex Shader.
const highp float Displacement = 200.0;



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

void main()
{
	// Pass through texture coordinates
	texcoord = inTexCoords;
	texcoord.y = 1.0 - texcoord.y;
	
	
	Vertex_World = (MMatrix * vec4(inVertex, 1.0)).xyz;

	//Water plane is places as xz coordinate, so that chaosUV need choose XY, and offset direction is Y
	highp vec2 chaosUV = (Vertex_World / WaterScale / LargeWaveScale).xz;  //Care dir
	
	VertexOffsetAfterChaos = MotionNWayChaos(chaosUV, WaterSpeed * LargeWaveSpeed, LargeWaves_HightTex, chaosWay).r;
	VertexOffsetAfterChaos = pow(VertexOffsetAfterChaos, 2.0);
	highp float VertexOffset = VertexOffsetAfterChaos;
	VertexOffset *= Displacement;
	
	highp vec3 VertexAfterOffset_Model = inVertex;
	VertexAfterOffset_Model.y += VertexOffset; //Care dir
	gl_Position = MVPMatrix * vec4(VertexAfterOffset_Model, 1.0);
	
	colorOut = MotionNWayChaos(chaosUV, WaterSpeed * LargeWaveSpeed, LargeWaves_HightTex, chaosWay);
	
	
	
	
	
	
	
	              
	
	highp vec3 ViewDir_ModelSpace = EyePosModel - inVertex.xyz;
	
	LightDir_WorldSpace = normalize(mat3(MMatrix) * LightDirModel);
	ViewDir_WorldSpace = normalize(mat3(MMatrix) * ViewDir_ModelSpace);
	
	highp vec3 NormalWorld = normalize(mat3(MMatrix_IT) * inNormal);
	highp vec3 TangentWorld = normalize(mat3(MMatrix_IT) * inTangent);
	highp vec3 BinormalWorld = normalize(mat3(MMatrix_IT) * inBiNormal);
	
	TangentToWorldMatrix = mat3(TangentWorld.x, TangentWorld.y, TangentWorld.z,
								BinormalWorld.x, BinormalWorld.y, BinormalWorld.z,
								NormalWorld.x, NormalWorld.y, NormalWorld.z);
	
	EyeToVertexDis = length(mat3(MMatrix) * EyePosModel - mat3(MMatrix) * inVertex.xyz);
	
}
