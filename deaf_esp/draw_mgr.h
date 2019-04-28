#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <memory>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

enum font_orient_t {
	FONT_LEFT,
	FONT_CENTER,
	FONT_RIGHT
};

#define WHITE(alpha)         D3DCOLOR_ARGB(alpha, 255, 255, 255)
#define BLACK(alpha)         D3DCOLOR_ARGB(alpha, 0, 0, 0)
#define RED(alpha)           D3DCOLOR_ARGB(alpha, 255, 0, 0)
#define GREEN(alpha)         D3DCOLOR_ARGB(alpha, 0, 128, 0)
#define LAWNGREEN(alpha)     D3DCOLOR_ARGB(alpha, 124, 252, 0)
#define BLUE(alpha)          D3DCOLOR_ARGB(alpha, 0, 200, 255)
#define DEEPSKYBLUE(alpha)   D3DCOLOR_ARGB(alpha, 0, 191, 255)
#define SKYBLUE(alpha)       D3DCOLOR_ARGB(alpha, 0, 122, 204)
#define YELLOW(alpha)        D3DCOLOR_ARGB(alpha, 255, 255, 0)
#define ORANGE(alpha)        D3DCOLOR_ARGB(alpha, 255, 165, 0)
#define DARKORANGE(alpha)    D3DCOLOR_ARGB(alpha, 255, 140, 0)
#define PURPLE(alpha)        D3DCOLOR_ARGB(alpha, 125, 0, 255)
#define CYAN(alpha)          D3DCOLOR_ARGB(alpha, 0, 255, 255)
#define PINK(alpha)          D3DCOLOR_ARGB(alpha, 255, 20, 147)
#define GRAY(alpha)          D3DCOLOR_ARGB(alpha, 128, 128, 128)
#define DARKGRAY(alpha)      D3DCOLOR_ARGB(alpha, 73, 73, 73)
#define DARKERGRAY(alpha)    D3DCOLOR_ARGB(alpha, 31, 31, 31)
#define EVENDARKERGRAY(alpha)    D3DCOLOR_ARGB(alpha, 20, 20, 20)

#define GetAlpha(col) (((col)&0xff000000)>>24)
#define GetRed(col) (((col)&0x00ff0000)>>16)
#define GetGreen(col) (((col)&0x0000ff00)>>8)
#define GetBlue(col) ((col)&0x000000ff)

class vec_t;

enum circle_type_t {
	full, half, quarter
};

struct vertex {
	float x, y, z, rhw;
	DWORD color;
};

class c_draw_mgr {
public:
	struct screen_t {
		float Width;
		float Height;
		float x_center;
		float y_center;
	};

	screen_t screen;

	IDirect3DDevice9* device;
	IDirect3DVertexBuffer9* vb;
	IDirect3DIndexBuffer9* ib;

	ID3DXSprite* sprite;
	IDirect3DTexture9* white_tex;

	struct font_t {
		ID3DXFont* findicators;
		ID3DXFont* ftabs;
		ID3DXFont* fnormal;
		ID3DXFont* fsmall;
		ID3DXFont* farrows;
		ID3DXFont* flog;
	};

	font_t fonts;

	void initialize( IDirect3DDevice9* device );
	void reset( IDirect3DDevice9* _device );

	int string_width( ID3DXFont* font, char* string );
	void line( int x, int y, int x2, int y2, DWORD color );
	void filled_box( int x, int y, int width, int height, DWORD color );
	void filled_box_outlined( int x, int y, int width, int height, DWORD colour, DWORD outlinecolor, int thickness = 1.0f );
	void bordered_box( int x, int y, int width, int height, DWORD color, int thickness = 1.0f );
	void bordered_box_outlined( int x, int y, int width, int height, DWORD colour, DWORD outlinecolor, int thickness = 1.0f );
	void gradient_box( int x, int y, int width, int height, DWORD color, DWORD color2, bool vertical );
	void gradient_box_outlined( int x, int y, int width, int height, int thickness, DWORD colour, DWORD color2, DWORD outlinecolor, bool vertical );
	void circle( int x, int y, int radius, int points, DWORD color );
	void filled_circle( int x, int y, int radius, int points, DWORD color );
	void filled_circle_outlined( int x, int y, int radius, int points, DWORD color, DWORD outlinecolor );

	void text( char *text, float x, float y, int orientation, ID3DXFont* pFont, bool bordered, DWORD color, DWORD bcolor );
	void string( float x, float y, int orientation, ID3DXFont* pFont, bool bordered, DWORD color, const char *input, ... );
	void message( char *text, float x, float y, ID3DXFont* pFont, int orientation );

	void draw_sprite( IDirect3DTexture9* tex, float x, float y, float resolution_x, float resolution_y, float scale_x, float scale_y, float rotation, DWORD color );

	void release( );
};

extern std::unique_ptr< c_draw_mgr > draw_mgr;