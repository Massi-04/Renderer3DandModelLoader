struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
};

cbuffer roba
{
	float4x4 mvp;
};

VSOutput main(float4 loc : Location, float4 col : Color)
{
	VSOutput res;
	res.Loc = mul(loc, mvp);
	res.Col = col;
	return res;
}