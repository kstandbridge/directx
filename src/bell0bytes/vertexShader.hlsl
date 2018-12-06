float4 main(float3 pos : POSITION) : SV_POSITION
{
	// transform the position into homogeneous coordinates (projective geometry)
	float4 homoPos = {pos.x, pos.y, pos.z, 1.0f};

	// return position
	return homoPos;
}