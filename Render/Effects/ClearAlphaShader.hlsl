Texture2D<float4> InputTexture0;
SamplerState InputSampler0;

cbuffer ShadowImageMeta : register(b0)
{
	float4 RcDirty : packoffset(c0);
	float4 ShadowPadding : packoffset(c1);
	float2 ImageSize : packoffset(c2);
	float alpha : packoffset(c2.z);
}

float4 PSMain(
	float4 pos   : SV_POSITION,
	float4 posScene : SCENE_POSITION,
	float4 uv0 : TEXCOORD0
) : SV_Target
{
	float4 input = InputTexture0.Sample(InputSampler0, uv0.xy);
	float2 tex_xy = uv0.xy;
	
	float x_pos = ImageSize[0] * tex_xy[0];
	float y_pos = ImageSize[1] * tex_xy[1];

	float pad_left = ShadowPadding[0];
	float pad_top = ShadowPadding[1];
	float pad_right = ShadowPadding[2];
	float pad_bottom = ShadowPadding[3];
	float n_left = RcDirty[0];
	float n_top = RcDirty[1];
	float n_right = RcDirty[2];
	float n_bottom = RcDirty[3];
	if (x_pos >= n_left && x_pos < n_right && y_pos >= n_top && y_pos < n_bottom)
	{
		if ((x_pos >= pad_left && x_pos < (ImageSize[0] - pad_right)) ||
			(y_pos >= pad_top && y_pos < (ImageSize[1] - pad_bottom)))
		{
			if (alpha != 0.0 && round(alpha * 255) == round(input.a * 255))
			{
				input.a = 0.0;
				//input.rgb *= input.a;
			}
			else {
				input.a = 1.0;
				//input.rgb *= input.a;
			}
		}
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