Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer cbData
{
	float4x4 World; 
	float4x4 View; 
	float4x4 Projection;

	float4 gDiffuseMtrl; 
	float4 gDiffuseLight; 

	float4 gAmbientMtrl;
	float4 gAmbientLight;

	float4 gSpecularMtrl;
	float4 gSpecularLight;
	float gSpecularPower;
	float3 gEyePosW;
	float3 gLightVecW;
	float3 gLightPos;
	float gLightRange;
	float3 gLightAtten;
	float pad;

	float spotCone;
};

struct VS_IN
{
	float4 posL		: POSITION;
	float3 normalL	: NORMAL;
	float2 texL		: TEXCOORD0;
	row_major float4x4 World : WORLD;
	float4 Color : COLOR;
	uint InstanceId : SV_InstanceID;
};

struct VS_OUT
{
	float4 Pos   : SV_POSITION;
	float3 Norm	 : NORMAL;
	float3 PosW  : POSITION;
	float2 Tex	 : TEXCOORD0;
	float4 Color : COLOR;
};

VS_OUT VS(VS_IN vIn)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = mul(vIn.posL, vIn.World);
	output.PosW = output.Pos.xyz;

	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	// Convert from local to world normal
	float3 normalW = mul(float4(vIn.normalL, 0.0f), vIn.World).xyz;
	output.Norm = normalize(normalW);

	float4 textureColour = txDiffuse.Sample(samLinear, vIn.texL);
	output.Tex = vIn.texL;
	output.Color = vIn.Color;

	return output;
}

float4 PS(VS_OUT pIn) : SV_Target
{
	pIn.Norm = normalize(pIn.Norm);
	float3 lightVec = normalize(gLightVecW);

	float3 toEye = normalize(gEyePosW - pIn.Pos.xyz);
	// Compute Colour
	// Compute the reflection vector.
	float3 r = reflect(-lightVec, pIn.Norm);
	// Determine how much (if any) specular light makes it
	// into the eye.
	float t = pow(max(dot(r, toEye), 0.0f), gSpecularPower);
	// Determine the diffuse light intensity that strikes the vertex.
	float s = max(dot(lightVec, pIn.Norm), 0.0f);

	if (s <= 0.0f)
	{
		t = 0.0f;
	}

	float4 textureColour = txDiffuse.Sample(samLinear, pIn.Tex);

	// Compute the ambient, diffuse, and specular terms separately.
	float3 spec = t*(gSpecularMtrl*gSpecularLight).rgb;
	float3 diffuse = s*textureColour*(gDiffuseMtrl*gDiffuseLight).rgb;
	float3 ambient = textureColour*(gAmbientMtrl * gAmbientLight).rgb;

	//ambient += ambient*pIn.Color;
	//diffuse += diffuse*pIn.Color;

	// Sum all the terms together and copy over the diffuse alpha.
	float4 Col;
	Col.rgb = ambient + diffuse + spec;
	Col.a = gDiffuseMtrl.a;
	//Col.rgb *= pow(max(dot(-toEye, lightVec), 0.0f), spotCone);

	clip(textureColour.a - 0.25f);

	return Col;
}

technique11 Render
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_4_0, VS() ) ); 
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS() ) );
	}
}
