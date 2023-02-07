struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
};

float4 main(VSOutput inData) : SV_TARGET
{
	return inData.Col;
}