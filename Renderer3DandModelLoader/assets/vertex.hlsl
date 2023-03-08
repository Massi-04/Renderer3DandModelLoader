struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
	float2 Coords : TextureCoords;
};

cbuffer roba
{
	float4x4 mvp;
};

VSOutput main(float4 loc : Location, float4 col : Color, float2 coords : TextureCoords)
{
	VSOutput res;
	res.Loc = mul(loc, mvp);
	res.Col = col;
	res.Coords = coords;
	return res;
}