struct VSOutput
{
	float4 Loc : SV_POSITION; // SV_POSITION
	float4 Col : Color;
	float2 Coords : TextureCoords;
};

Texture2D Texture0;
SamplerState Sampler0;

float4 main(VSOutput inData) : SV_TARGET
{
	return Texture0.Sample(Sampler0, inData.Coords) * inData.Col;
}