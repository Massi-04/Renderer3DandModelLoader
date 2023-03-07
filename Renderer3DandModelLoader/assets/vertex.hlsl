struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
	float3 Norm : Normal;
	float2 Coords : TextureCoords;
};

cbuffer roba
{
	float4x4 mvp;
};

VSOutput main(float4 loc : Location, float4 col : Color, float3 norm : Normal, float2 coords : TextureCoords)
{
	VSOutput res;
	res.Loc = mul(loc, mvp);
	res.Col = col;
	res.Norm = norm;
	res.Coords = coords;
	return res;
}