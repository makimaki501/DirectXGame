cbuffer cbuff0:register(b0)
{
	float4 color;//êF(RGBA)
}

float4 PSmain(float4 pos:SV_POSITION):SV_TARGET
{
	return color;
}