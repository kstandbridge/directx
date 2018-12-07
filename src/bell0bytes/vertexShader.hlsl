cbuffer ConstantColourPositionBuffer
{
	float3 xyz;
	float3 rgb;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

VertexOut main(float3 pos : POSITION, float3 col : COLOR)
{
	// create a VertexOut structure
	VertexOut vertexOutput;

	// transform the position into homogeneous coordinates (projective geometry)
	float4 outputPos = { pos.x, pos.y, pos.z, 1.0f };
	vertexOutput.position = outputPos;

	// set the colour (set full alpha)
	float4 outputCol = { col.r, col.g, col.b, 1.0f };
	vertexOutput.colour = outputCol;

	// return position
	return vertexOutput;
}