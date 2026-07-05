#include "Sprite.h"

using namespace DirectX;

// The constant buffer's layout as seen by the GPU (see Sprite.hlsl's
// TransformBuffer). D3D11 constant buffers must be sized in multiples of
// 16 bytes; a single 4x4 matrix already satisfies that.
struct TransformConstants
{
    XMMATRIX mvp;
    XMFLOAT2 uvOffset;
    XMFLOAT2 uvScale;
    XMFLOAT4 tint;
};

bool Sprite::Init(ID3D11Device* device)
{
    // Unit quad in local space, spanning (0,0) to (1,1). Draw() scales and
    // translates this per-sprite instead of rebuilding geometry every call.
    SpriteVertex vertices[] = {
        { 0.0f, 0.0f, 0.0f,  0.0f, 0.0f }, // top-left
        { 1.0f, 0.0f, 0.0f,  1.0f, 0.0f }, // top-right
        { 1.0f, 1.0f, 0.0f,  1.0f, 1.0f }, // bottom-right
        { 0.0f, 1.0f, 0.0f,  0.0f, 1.0f }, // bottom-left
    };
    UINT16 indices[] = { 0, 1, 2, 0, 2, 3 };

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vbData = { vertices };
    if (FAILED(device->CreateBuffer(&vbDesc, &vbData, &m_vertexBuffer)))
        return false;

    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = sizeof(indices);
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA ibData = { indices };
    if (FAILED(device->CreateBuffer(&ibDesc, &ibData, &m_indexBuffer)))
        return false;

    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.Usage = D3D11_USAGE_DYNAMIC; // rewritten every draw call (position changes each frame)
    cbDesc.ByteWidth = sizeof(TransformConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &m_constantBuffer)))
        return false;

    return true;
}

void Sprite::Draw(ID3D11DeviceContext* context, Shader& shader, Texture& texture,
                   float x, float y, float width, float height,
                   int screenWidth, int screenHeight,
                   const RECT& sourceRectPixels, bool flipHorizontal, XMFLOAT4 tint)
{
    // Model matrix: scale the unit quad to the sprite's pixel size, then
    // move it to its screen position. A horizontal flip negates the
    // width scale (mirroring the quad) and shifts the translation by
    // +width so the sprite's on-screen footprint (x to x+width) stays in
    // the same place instead of mirroring off to one side.
    float scaleX = flipHorizontal ? -width : width;
    float translateX = flipHorizontal ? x + width : x;
    XMMATRIX model = XMMatrixScaling(scaleX, height, 1.0f) * XMMatrixTranslation(translateX, y, 0.0f);

    // Orthographic projection mapping screen pixel space - (0,0) top-left,
    // +Y down - directly to normalized device coordinates. Passing
    // bottom=screenHeight, top=0 (instead of the usual bottom<top) is what
    // flips Y so pixel-space "down" matches NDC "up" correctly.
    XMMATRIX projection = XMMatrixOrthographicOffCenterLH(
        0.0f, static_cast<float>(screenWidth),
        static_cast<float>(screenHeight), 0.0f,
        0.0f, 1.0f
    );

    // Convert the frame's pixel-space rectangle within the texture into
    // normalized 0..1 UV offset/scale, so the vertex shader can remap the
    // quad's baked-in 0..1 UVs onto just this one frame of the sheet.
    float texWidth = static_cast<float>(texture.GetWidth());
    float texHeight = static_cast<float>(texture.GetHeight());

    TransformConstants constants;
    // HLSL cbuffer matrices are column-major by default; XMMatrixTranspose
    // converts from XMMATRIX's row-major storage to match.
    constants.mvp = XMMatrixTranspose(model * projection);
    constants.uvOffset = XMFLOAT2(sourceRectPixels.left / texWidth, sourceRectPixels.top / texHeight);
    constants.uvScale = XMFLOAT2(
        (sourceRectPixels.right - sourceRectPixels.left) / texWidth,
        (sourceRectPixels.bottom - sourceRectPixels.top) / texHeight
    );
    constants.tint = tint;

    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &constants, sizeof(constants));
    context->Unmap(m_constantBuffer.Get(), 0);

    // Bound to both stages: the vertex shader reads mvp/uvOffset/uvScale,
    // and the pixel shader reads tint - an unbound stage would read all
    // zeros for this buffer instead of failing loudly, silently making
    // every draw fully transparent.
    ID3D11Buffer* cb = m_constantBuffer.Get();
    context->VSSetConstantBuffers(0, 1, &cb);
    context->PSSetConstantBuffers(0, 1, &cb);

    shader.Bind(context);
    texture.Bind(context, 0);

    UINT stride = sizeof(SpriteVertex);
    UINT offset = 0;
    ID3D11Buffer* vb = m_vertexBuffer.Get();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(6, 0, 0);
}
