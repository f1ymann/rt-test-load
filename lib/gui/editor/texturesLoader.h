#ifndef TEXTURES_LOADER_H
#define TEXTURES_LOADER_H

#include <vector>
#include <algorithm>

#include <imgui.h>
#ifdef _WIN32
#include <d3d11.h>
#else
#include <GLFW/glfw3.h>
#endif // WIN32


namespace rt3gui {
	struct ImTexture
	{
#ifdef _WIN32
		ID3D11ShaderResourceView* TextureID = NULL;
#else
		GLuint TextureID = 0;
#endif // _WIN32

		int    Width = 0;
		int    Height = 0;
	};

	class TexturesLoader {
	public:
#ifdef _WIN32
		TexturesLoader(ID3D11Device* _g_pd3dDevice);
#else
		TexturesLoader();
#endif // _WIN32
		~TexturesLoader();
		ImTextureID Application_CreateTexture(const void* data, int width, int height);
		ImTextureID Application_LoadTexture(const char* path);
		std::vector<ImTexture>::iterator Application_FindTexture(ImTextureID texture);
		void Application_DestroyTexture(ImTextureID texture);
		int Application_GetTextureWidth(ImTextureID texture);
		int Application_GetTextureHeight(ImTextureID texture);


	private:
		std::vector<ImTexture> g_Textures;
#ifdef _WIN32
		ID3D11Device* g_pd3dDevice;
#endif
	};
} //RT3Gui


#endif