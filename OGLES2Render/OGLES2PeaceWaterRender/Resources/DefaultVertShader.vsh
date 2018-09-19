#version 100
uniform highp mat4 MVPMatrix;
uniform highp mat4 MMatrix_IT;
uniform highp mat4 MMatrix;
uniform mediump vec3 LightDirModel;
uniform highp vec3 EyePosModel;
uniform highp float _Time;

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

varying mediump vec4 NormalCoords;
varying mediump vec4 NormalAfterDistortion_World;

varying mediump vec4 ReflectionUV;
varying mediump vec4 RefractionUV;

const highp float VertexDisplacement = 0.5;
const highp vec4 Amplitude = vec4(0.8, 0.2, 0.25, 0.25);
const highp vec4 Frequency = vec4(0.1, 0.25, 0.2, 0.245) / 10.0;
const highp vec4 Steepness = vec4(0.2, 0.2, 0.0, 0.0);
const highp vec4 Speed = vec4(4.0, 2.0, 1.0, 1.0);
const highp vec4 DirectionAB = vec4(0.85, 0.3, 0.25, 0.25);
const highp vec4 DirectionCD = vec4(-0.3, -0.9, 0.5, 0.5);

const highp vec4 NormalMapSampleDirSpeed = vec4(1.0, 30.0, 20.0, -20.0); //xy Dir, zw Speed
const highp vec4 NormalMapSampleScale = vec4(0.002, 0.002, 0.002, 0.002);

highp vec3 GerstnerOffset4(highp vec2 xzVtx,  //WaterPlane xz
						   highp vec4 steepness,
						   highp vec4 amp,
						   highp vec4 freq,
						   highp vec4 speed,
						   highp vec4 dirAB,
						   highp vec4 dirCD)
{
	highp vec3 offsets;
	
	highp vec4 AB = steepness.xxyy * amp.xxyy * dirAB.xyzw;
	highp vec4 CD = steepness.zzww * amp.zzww * dirCD.xyzw;
	
	highp vec4 dotABCD = freq.xyzw * vec4(dot(dirAB.xy, xzVtx), dot(dirAB.zw, xzVtx), dot(dirCD.xy, xzVtx), dot(dirCD.zw, xzVtx));
	highp vec4 TIME = _Time * speed;
	
	highp vec4 COS = cos(dotABCD + TIME);
	highp vec4 SIN = sin(dotABCD + TIME);
	
	offsets.x = dot(COS, vec4(AB.xz, CD.xz));
	offsets.z = dot(COS, vec4(AB.yw, CD.yw));
	offsets.y = dot(SIN, amp);
	
	return offsets;
}

highp vec3 GerstnerNormal4 (highp vec2 xzVtx,
							highp vec4 amp,
							highp vec4 freq,
							highp vec4 speed,
							highp vec4 dirAB,
							highp vec4 dirCD) 
{
	highp vec3 nrml = vec3(0, 2.0, 0);
	
	highp vec4 AB = freq.xxyy * amp.xxyy * dirAB.xyzw;
	highp vec4 CD = freq.zzww * amp.zzww * dirCD.xyzw;
	
	highp vec4 dotABCD = freq.xyzw * vec4(dot(dirAB.xy, xzVtx), dot(dirAB.zw, xzVtx), dot(dirCD.xy, xzVtx), dot(dirCD.zw, xzVtx));
	highp vec4 TIME = _Time * speed;
	
	highp vec4 COS = cos (dotABCD + TIME);
	
	nrml.x -= dot(COS, vec4(AB.xz, CD.xz));
	nrml.z -= dot(COS, vec4(AB.yw, CD.yw));
	
	nrml.xz *= VertexDisplacement;
	nrml = normalize (nrml);

	return nrml;			
}	

void main()
{
	// Pass through texture coordinates
	texcoord = inTexCoords;
	texcoord.y = 1.0 - texcoord.y;
	highp vec3 Vertex_Model = inVertex;	
	
	Vertex_World = (MMatrix * vec4(Vertex_Model, 1.0)).xyz;
	highp vec3 vertexForAni = Vertex_World.xzz;
	highp vec3 offsets = GerstnerOffset4(vertexForAni.xz, Steepness, Amplitude, Frequency, Speed, DirectionAB, DirectionCD);
	highp vec3 normal = GerstnerNormal4 (vertexForAni.xz, Amplitude,Frequency, Speed, DirectionAB, DirectionCD);
	
	Vertex_Model.xyz += offsets;
	highp vec2 tileableUV = Vertex_World.xz;
	
	NormalCoords.xyzw = (tileableUV.xyxy + _Time / 20.0 * NormalMapSampleDirSpeed.xyzw) * NormalMapSampleScale;
	
	//gl_Position = MVPMatrix * vec4(Vertex_Model, 1.0);
	gl_Position = MVPMatrix * vec4(inVertex, 1.0);
	colorOut = inVertex / 50.0 + 1.0;
	
	NormalAfterDistortion_World = vec4(normal, 1.0);
	       
	
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
	
	ReflectionUV = vec4(vec2(gl_Position.x, -gl_Position.y) / gl_Position.w * 0.5 + 0.5, gl_Position.z, gl_Position.w);
	RefractionUV = vec4(vec2(gl_Position.x, gl_Position.y) / gl_Position.w * 0.5 + 0.5, gl_Position.z, gl_Position.w);
}
