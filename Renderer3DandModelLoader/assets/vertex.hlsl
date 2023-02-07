struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
};

VSOutput main(float4 loc : Location, float4 col : Color)
{
	VSOutput res;
	res.Loc = loc;
	res.Col = col;
	return res;
}