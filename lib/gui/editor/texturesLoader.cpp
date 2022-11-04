#include <texturesLoader.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
extern "C" {
#include <stb_image.h>
}

#ifdef _WIN32
rt3gui::TexturesLoader::TexturesLoader(ID3D11Device* _g_pd3dDevice) {
    g_Textures.clear();
    g_pd3dDevice = _g_pd3dDevice;
}
#else
rt3gui::TexturesLoader::TexturesLoader() {
    g_Textures.clear();
}
#endif // _WIN32



rt3gui::TexturesLoader::~TexturesLoader() {

}

ImTextureID rt3gui::TexturesLoader::Application_CreateTexture(const void* data, int width, int height)
{
    g_Textures.resize(g_Textures.size() + 1);
    ImTexture& texture = g_Textures.back();

#ifdef _WIN32
    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    
    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &texture.TextureID);
    pTexture->Release();

    texture.Width = width;
    texture.Height = height;
    return reinterpret_cast<ImTextureID>(static_cast<ID3D11ShaderResourceView*>(texture.TextureID));
#else
    // Upload texture to graphics system
    GLint last_texture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &texture.TextureID);
    glBindTexture(GL_TEXTURE_2D, texture.TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, last_texture);

    texture.Width = width;
    texture.Height = height;
    // Definitions of common types
#ifndef intptr_t
    #ifdef _WIN64
        typedef __int64          intptr_t;
    #else
        typedef int              intptr_t;
    #endif
#endif        
    return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture.TextureID));
#endif // _WIN32



}

ImTextureID rt3gui::TexturesLoader::Application_LoadTexture(const char* path)
{
    int width = 0, height = 0, component = 0;
    if (auto data = stbi_load(path, &width, &height, &component, 4))
    {
        auto texture = Application_CreateTexture(data, width, height);
        stbi_image_free(data);
        return texture;
    }
    else
        return nullptr;
}

std::vector<rt3gui::ImTexture>::iterator rt3gui::TexturesLoader::Application_FindTexture(ImTextureID texture)
{
#ifdef _WIN32
    auto textureID = (reinterpret_cast<ID3D11ShaderResourceView*>(texture));
#else
    auto textureID = static_cast<GLuint>(reinterpret_cast<intptr_t>(texture));
#endif // _WIN32

    return std::find_if(g_Textures.begin(), g_Textures.end(), [textureID](ImTexture& texture)
        {
            return texture.TextureID == textureID;
        });
}

void rt3gui::TexturesLoader::Application_DestroyTexture(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt == g_Textures.end())
        return;

#ifdef _WIN32
    reinterpret_cast<ID3D11ShaderResourceView*>(texture)->Release();
#else
    glDeleteTextures(1, &textureIt->TextureID);
#endif // _WIN32

    g_Textures.erase(textureIt);
}

int rt3gui::TexturesLoader::Application_GetTextureWidth(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Width;
    return 0;
}

int rt3gui::TexturesLoader::Application_GetTextureHeight(ImTextureID texture)
{
    auto textureIt = Application_FindTexture(texture);
    if (textureIt != g_Textures.end())
        return textureIt->Height;
    return 0;
}
