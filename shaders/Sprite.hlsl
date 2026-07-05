// Vertex + pixel shader pair for drawing a single textured 2D quad (sprite).

cbuffer TransformBuffer : register(b0)
{
    // Combined model * orthographic-projection matrix, computed on the CPU
    // per-sprite so pixel-space coordinates map directly to screen pixels.
    matrix mvp;

    // Remaps the quad's 0..1 UVs to a single frame's rectangle within a
    // sprite sheet texture, so the same unit quad can draw any frame.
    float2 uvOffset;
    float2 uvScale;

    // Multiplied into the sampled color - lets one grayscale/white source
    // texture (e.g. the HUD's digit font) be recolored per-draw instead
    // of needing a separate texture per color.
    float4 tint;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.position = mul(float4(input.position, 1.0f), mvp);
    output.uv = uvOffset + input.uv * uvScale;
    return output;
}

Texture2D spriteTexture : register(t0);
SamplerState spriteSampler : register(s0);

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return spriteTexture.Sample(spriteSampler, input.uv) * tint;
}
