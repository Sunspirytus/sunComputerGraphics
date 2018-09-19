uniform mediump vec3 diffuseColor;
uniform mediump sampler2D diffuseMap;

varying mediump vec3 lightDirInModel;
varying mediump vec3 viewDirInModel;
varying mediump vec3 normalInModel;
varying mediump vec2 uv;

void main()
{
	mediump vec3 lightDirInModel_normalized = normalize(lightDirInModel);
	mediump vec3 viewDirInModel_normalized = normalize(viewDirInModel);
	mediump vec3 normalInModel_normalized = normalize(normalInModel);
	
	mediump vec2 texCoord = uv;
	texCoord.y = 1.0 - uv.y;
	mediump vec3 Albedo = texture2D(diffuseMap, texCoord).rgb;
	
	mediump vec3 diffuse = Albedo * max(dot(normalInModel_normalized, lightDirInModel_normalized), 0.0);
	mediump vec3 specularColor = vec3(0.5, 0.5, 0.5);
	mediump vec3 halfDir = normalize(lightDirInModel_normalized + viewDirInModel_normalized);
	mediump vec3 specular = specularColor * max(dot(normalInModel_normalized, halfDir), 0.0);

	gl_FragColor = vec4(diffuse + specular, 1.0);
}