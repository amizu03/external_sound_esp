#pragma once
#include <memory>
#include <d3d9.h>
#include <d3dx9.h>

class c_renderer {
public:
	bool renderer_init = false;
	IDirect3D9* d3d;
	IDirect3DDevice9* d3d_device;
	D3DPRESENT_PARAMETERS params;
	D3DVIEWPORT9 viewport;

	void init( void );
	void reset( void );
	void render( void );
};

extern std::unique_ptr< c_renderer > renderer;