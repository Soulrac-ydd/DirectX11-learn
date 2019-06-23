
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float4 Color0 : COLOR0;
    float4 Color1 : COLOR1;
};

VS_OUTPUT VS(float4 inPos : POSITION, float4 inColor0 : COLOR0, float4 inColor1 : COLOR1)
{
    VS_OUTPUT output;

    output.Pos = inPos;
    output.Color0 = inColor0;
    output.Color1 = inColor1;

    return output;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return input.Color0 * input.Color1;
}
