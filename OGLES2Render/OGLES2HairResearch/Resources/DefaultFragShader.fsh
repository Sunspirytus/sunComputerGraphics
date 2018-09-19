#version 100
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specFlowMap;
uniform lowp float Pass;

varying mediump vec3 NormalModel;
varying mediump vec2 TexCoords1;
varying mediump mat3 ModelToTangentMatrix; 
varying mediump vec3 LightDir_TangentSpace;
varying mediump vec3 ViewDir_TangentSpace;


const mediump vec3 _Spec1Color = vec3(1.04, 0.04, 0.04);
const mediump float _Spec1EX = 72.0;
const mediump float _Spec1Shift = 0.1;
const mediump vec3 _Spec2Color = vec3(0.12, 1.12, 0.12);
const mediump float _Spec2EX = 48.0;
const mediump float _Spec2Shift = 0.2;

const mediump vec3 _LightColor = vec3(1.0, 1.0, 1.0);
const mediump float _SpecBlinnEX = 108.0;

mediump float lerp(mediump float a, mediump float b, mediump float w) {
  return a + w*(b-a);
}

mediump vec3 UnpackScaleNormal(mediump vec4 packedNormal){
	mediump vec3 normal;
	normal.xyz = packedNormal.xyz * 2.0 - 1.0;
	return normal;
}

void main()
{

	highp vec2 texcoords1 = TexCoords1;
	texcoords1.y = 1.0 - texcoords1.y;
	
	highp vec4 Albedo = texture2D(diffuseMap, texcoords1).rgba;
	if(Pass == 1.0){
		if(Albedo.a <0.8){
			discard;
		}		
	}	
	
	highp vec3 Normal_TangentSpace_normalized = normalize(UnpackScaleNormal(texture2D(normalMap, texcoords1)));

	highp vec3 tanFlow = texture2D(specFlowMap, texcoords1).rgb * 2.0 - 1.0;
	highp vec3 normalTangent = normalize(ModelToTangentMatrix * NormalModel);
	highp vec3 tanDir1 = normalize(tanFlow + normalTangent * _Spec1Shift);
	highp vec3 tanDir2 = normalize(tanFlow + normalTangent * _Spec2Shift);
	highp vec3 halfDirTangent = normalize(LightDir_TangentSpace + ViewDir_TangentSpace);
	highp float nh = max(dot(Normal_TangentSpace_normalized, halfDirTangent), 0.0);
	highp float nl = max(dot(Normal_TangentSpace_normalized, LightDir_TangentSpace), 0.0);
	highp float diffuseTerm = nl;
	highp float specularTerm = pow(nh , _SpecBlinnEX);
	highp float th1 = dot(tanDir1, halfDirTangent);
	highp float th2 = dot(tanDir2, halfDirTangent);
	highp vec3 spec1 =  _Spec1Color * pow(sqrt(1.0 - th1 * th1), _Spec1EX);
	highp vec3 spec2 =  _Spec2Color * pow(sqrt(1.0 - th2 * th2), _Spec2EX);
	highp vec3 specBlinn = clamp(0.0, 1.0, nl) * _LightColor * specularTerm;
	highp vec3 diffuseColor = Albedo.rgb * vec3(0.0, 0.0, 0.0);
	highp vec3 color = vec3(0.0, 0.0, 0.0);
	color += diffuseColor * diffuseTerm;
	color += spec1;
	color += spec2;
	color += specBlinn;
	
	gl_FragColor = vec4(color, Albedo.a);
}