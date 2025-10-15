Texture2D<float4> InputTexture0;
SamplerState InputSampler0;

cbuffer FixMeta : register(b0)
{
	float4 TextColor : packoffset(c0);
}

inline float cosinColorVec(float r1, float g1, float b1, float r2, float g2, float b2)
{

}

float4 PSMain(
	float4 pos   : SV_POSITION,
	float4 posScene : SCENE_POSITION,
	float4 uv0 : TEXCOORD0
) : SV_Target
{
	float4 input = InputTexture0.Sample(InputSampler0, uv0.xy);
	float2 tex_xy = uv0.xy;

	float alpha = TextColor[0];
	float b = TextColor[1];
	float g = TextColor[2];
	float r = TextColor[3];

	if ((input.r > 0.0 || input.g > 0.0 || input.b > 0.0) && input.a == 0.0) {
		float r1 = r;
		float g1 = g;
		float b1 = b;
		float r2 = input.r;
		float g2 = input.g;
		float b2 = input.b;
		float l1 = sqrt(r1*r1 + g1*g1 + b1*b1);
		float l2 = sqrt(r2*r2 + g2*g2 + b2*b2);
		float cosval = min((r1*r2 + g1*g2 + b1*b2) / (l1*l2), 1.0);
		float sar = min(r2 / r1, 1.0);
		float sag = min(g2 / g1, 1.0);
		float sab = min(b2 / b1, 1.0);
		float fixAlpha = ((sar + sag + sab) / 3);

		if (cosval > 0.6)
		{
			input.r = (input.r / sar);
			input.g = (input.g / sag);
			input.b = (input.b / sab);
			input.a = fixAlpha;
		}
		else {
			input.r = 0;
			input.g = 0;
			input.b = 0;
			input.a = 0;
		}
		//input.a = min(sab, min(sar, sag));
		//input.a = fixAlpha + log2(min(cosv, 1.0) + 1.0)*(alpha * (1 - cosv));

		//input.a = fixAlpha + cosv*alpha * (1 - cosv);
		//input.a =  log2(min(cosv, 1.0) + 1.0);
	}

	return input;
}

/*
export float4 PSMain_Function(
float4 input0 : INPUT0
)
{
input0.rgb *= input0.a;

return input0;
}*/