attribute mediump vec3 inVertex;

uniform mediump mat4 ModelMatrix;
uniform highp mat4 MVPMatrix;
uniform mediump float WaterHeight;		//Assume water always lies on the y-axis

varying mediump vec3 EyeDir;
varying mediump float VertexHeight;


void main()
{
	highp vec3 vertexModel = inVertex;
	//vertexModel.y += 300.0;
	EyeDir = -vertexModel;
	gl_Position = MVPMatrix * vec4(vertexModel, 1.0);
	
	// Calculate the vertex's distance ABOVE water surface.
	mediump float vVertexHeight = (ModelMatrix * vec4(vertexModel,1.0)).y;
	VertexHeight = vVertexHeight - WaterHeight;
}