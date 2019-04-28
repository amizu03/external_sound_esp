#include <deque>
#include <ctime>
#include <chrono>
#include "renderer.h"
#include "draw_mgr.h"
#include "sdk.h"

extern std::uint64_t last_render_time;
extern std::deque< vec_t > footsteps_synced;
extern std::deque< vec_t > footsteps;
extern std::uint64_t vmatrix;
extern RECT rect;
extern std::uint64_t client;
extern std::uint64_t engine;
extern HWND window;
extern HWND overlay;
extern void* handle;

void c_renderer::init( void ) {
	memset( &params, 0, sizeof( params ) );

	params.EnableAutoDepthStencil = true;
	params.AutoDepthStencilFormat = D3DFMT_D16;
	params.Windowed = true;
	params.BackBufferCount = 1;
	params.BackBufferFormat = D3DFMT_A8R8G8B8;
	params.BackBufferWidth = rect.right - rect.left;
	params.BackBufferHeight = rect.bottom - rect.top;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	params.hDeviceWindow = overlay;

	d3d = Direct3DCreate9( D3D_SDK_VERSION );

	if ( !d3d )
		return;

	if ( d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, overlay, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &d3d_device ) != D3D_OK )
		return;

	if ( !d3d_device )
		return;
}

void c_renderer::reset( void ) {
	if ( d3d_device ) {
		params.BackBufferWidth = rect.left - rect.right;
		params.BackBufferHeight = rect.bottom - rect.top;

		d3d_device->Reset( &params );
		draw_mgr->initialize( d3d_device );
	}
}

void c_renderer::render( void ) {
	static bool init_once = false;

	if ( !init_once ) {
		init( );
		draw_mgr->initialize( d3d_device );
		init_once = true;
	}

	if ( !d3d_device )
		return;

	d3d_device->Clear( 0, nullptr, D3DCLEAR_TARGET, 0, 1.0f, 0 );
	d3d_device->BeginScene( );

	draw_mgr->reset( d3d_device );

	vmatrix_t matrix;
	ReadProcessMemory( handle, ( void* ) vmatrix, &matrix, sizeof( vmatrix_t ), nullptr );

	static auto draw_3d_circle = [ & ] ( vec_t pos, float rad, vmatrix_t matrix, int alpha ) {
		constexpr auto circle_step = 3.1412596535f / 75.0f;
		constexpr auto tau = 3.1412596535f * 2.0f;

		for ( float a = 0; a < tau; a += circle_step ) {
			auto start = vec_t( rad * std::cosf( a ) + pos.x, rad * std::sinf( a ) + pos.y, pos.z + 2.0f );
			auto end = vec_t( rad * std::cosf( a + circle_step ) + pos.x, rad * std::sinf( a + circle_step ) + pos.y, pos.z + 2.0f );
			auto start_scrn = vec_t( );
			auto end_scrn = vec_t( );

			if ( world_to_screen( start, start_scrn, matrix ) && world_to_screen( end, end_scrn, matrix ) )
				draw_mgr->line( start_scrn.x, start_scrn.y, end_scrn.x, end_scrn.y, RED( alpha ) );
		}
	};

	if ( GetForegroundWindow( ) == window ) {
		auto t = time( nullptr );

		if ( footsteps_synced.size( ) && t - last_render_time < 1 ) {
			for ( auto& footstep : footsteps_synced ) {
				draw_3d_circle( footstep, 35.0f, matrix, 255 );
			}
		}
	}

	d3d_device->EndScene( );
	d3d_device->Present( nullptr, nullptr, nullptr, nullptr );
}

std::unique_ptr< c_renderer > renderer = std::make_unique< c_renderer >( );