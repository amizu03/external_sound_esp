#include <Windows.h>
#include <ctime>
#include <chrono>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstdlib>
#include <process.h>
#include <cstdint>
#include <string>
#include <Uxtheme.h>
#include <dwmapi.h>
#include <vector>
#include <TlHelp32.h>
#include <iostream>
#include <deque>
#include "renderer.h"
#include "sdk.h"

#pragma comment ( lib, "dwmapi.lib" )

std::uint32_t last_render_time = 0;
std::deque< vec_t > footsteps { };
std::deque< vec_t > footsteps_synced { };
int sounds_size = 0;
std::uint32_t vmatrix = 0;
std::uint32_t local_player = 0;
std::uint32_t entity_list = 0;
std::uint32_t sound_list_array_base = 0;
std::uint32_t sound_list = 0;
std::uint32_t hk = 0;
std::uint32_t sound_list_synced = 0;
std::uint32_t fsn = 0;
std::uint32_t s_getactivesounds = 0;
std::uint32_t client = 0;
std::uint32_t engine = 0;
std::uint32_t game_event_mgr = 0;
std::uint32_t fire_event_clientside = 0;
unsigned long pid = 0;
void* handle = nullptr;
HWND window = nullptr;
HWND overlay = nullptr;
WNDCLASSEXA wndclass;
MSG msg;
RECT rect { };

std::string classname;
std::string wndname;

void run_find_window( void ) {
	while ( true ) {
		window = FindWindowA( nullptr, "Counter-Strike: Global Offensive" );

		if ( !window )
			exit( 0 );

		Sleep( 1 );
	}
}

void run_get_sound_data( void ) {
	while ( true ) {
		std::uint32_t local = 0;
		ReadProcessMemory( handle, ( void* ) local_player, &local, 4, nullptr );

		std::uint32_t local_team = 0;
		ReadProcessMemory( handle, ( void* ) ( local + 0xF4 ), &local_team, 4, nullptr );

		ReadProcessMemory( handle, ( void* ) ( sound_list_synced + 12 ), &sounds_size, 4, nullptr );
		ReadProcessMemory( handle, ( void* ) sound_list_synced, &sound_list_array_base, 4, nullptr );

		auto sounds = new snd_info_t[ sounds_size ];
		ReadProcessMemory( handle, ( void* ) sound_list_array_base, &sounds[ 0 ], sizeof( snd_info_t ) * sounds_size, nullptr );

		if ( footsteps.size( ) ) {
			footsteps_synced = footsteps;
			last_render_time = time( nullptr );
		}

		footsteps.clear( );

		for ( auto i = 0; i < sounds_size; i++ ) {
			auto sound = sounds[ i ];
			
			vec_t snd_origin;
			ReadProcessMemory( handle, ( void* ) sound.p_origin, &snd_origin, sizeof( vec_t ), nullptr );

			if ( !sound.from_server || sound.sound_src == 0 || sound.sound_src > 64 || snd_origin == vec_t( 0.0f, 0.0f, 0.0f ) )
				continue;

			std::uint32_t e = 0;
			ReadProcessMemory( handle, ( void* ) ( entity_list + ( sound.sound_src - 1 ) * 16 ), &e, 4, nullptr );

			if ( !e )
				continue;

			std::uint32_t health = 0;
			ReadProcessMemory( handle, ( void* ) ( e + 0x100 ), &health, 4, nullptr );

			std::uint32_t team = 0;
			ReadProcessMemory( handle, ( void* ) ( e + 0xF4 ), &team, 4, nullptr );

			if ( team == local_team || !health || e == local_player )
				continue;

			footsteps.push_back( snd_origin );
		}

		delete[ ] sounds;

		Sleep( 1 );
	}
}

std::uint32_t __stdcall wndproc( HWND wnd, std::uint32_t msg, std::uint32_t wparam, std::uint32_t lparam ) {
	switch ( msg ) {
		case WM_PAINT:			
			renderer->render( );
			break;
		case WM_SIZE:
			renderer->reset( );
			break;
		case WM_DESTROY:
			PostQuitMessage( 1 );
			break;
		default:
			return DefWindowProcA( wnd, msg, wparam, lparam );
	}

	return 0;
}

std::string gen_random_str( const int len ) {
	std::string str;
	char alphanum[ ] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	for ( int i = 0; i < len; ++i )
		str += alphanum[ rand( ) % ( sizeof( alphanum ) - 1 ) ];

	return str;
}

std::uint32_t get_module( const char* mod_name ) {
	std::uint32_t mod = 0;
	auto snap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid );

	if ( ( int ) snap != -1 ) {
		MODULEENTRY32 mod_entry;
		mod_entry.dwSize = sizeof( mod_entry );

		if ( Module32First( snap, &mod_entry ) ) {
			do {
				if ( !strcmp( mod_entry.szModule, mod_name ) ) {
					mod = ( std::uint32_t ) mod_entry.modBaseAddr;
					break;
				}
			} while ( Module32Next( snap, &mod_entry ) );
		}
	}

	CloseHandle( snap );

	return mod;
}

bool cmp( std::uint8_t* data, std::uint8_t* mask, const char* sz ) {
	for ( ; *sz; ++sz, ++data, ++mask )
		if ( *sz == 'x' && *data != *mask )
			return false;

	return !*sz;
}

std::uint32_t find_pattern_client( const char* sig ) {
	std::string mask = "";
	std::vector< std::uint8_t > bytes { };

	for ( int i = 0; i < strlen( sig ); i++ ) {
		if ( sig[ i ] == ' ' ) {
			continue;
		}
		else if ( sig[ i ] == '?' ) {
			mask += '?';
			bytes.push_back( 0x00 );
		}
		else {
			auto cur_bytes = new char[ 3 ];
			cur_bytes[ 0 ] = sig[ i ];
			cur_bytes[ 1 ] = sig[ i + 1 ];
			cur_bytes[ 2 ] = '\0';
			bytes.push_back( std::stoi( cur_bytes, 0, 16 ) );
			i++;
			mask += 'x';
			delete[ ] cur_bytes;
		}
	}

	auto olly_sig = new char[ bytes.size( ) + 1 ];
	std::memcpy( olly_sig, &bytes[ 0 ], bytes.size( ) );
	olly_sig[ bytes.size( ) + 1 ] = '\0';

	auto data = new std::uint8_t[ 25000000 ];
	ReadProcessMemory( handle, ( void* ) client, &data[ 0 ], 25000000, nullptr );

	for ( std::uint32_t i = 0; i < 25000000; i++ ) {
		if ( cmp( ( std::uint8_t* ) ( &data[ 0 ] + i ), ( std::uint8_t* ) olly_sig, mask.c_str( ) ) ) {
			delete[ ] data;
			delete[ ] olly_sig;
			return client + i;
		}
	}

	delete[ ] data;
	delete[ ] olly_sig;
	return 0;
}

std::uint32_t find_pattern_engine( const char* sig ) {
	std::string mask = "";
	std::vector< std::uint8_t > bytes { };

	for ( int i = 0; i < strlen( sig ); i++ ) {
		if ( sig[ i ] == ' ' ) {
			continue;
		}
		else if ( sig[ i ] == '?' ) {
			mask += '?';
			bytes.push_back( 0x00 );
		}
		else {
			auto cur_bytes = new char[ 3 ];
			cur_bytes[ 0 ] = sig[ i ];
			cur_bytes[ 1 ] = sig[ i + 1 ];
			cur_bytes[ 2 ] = '\0';
			bytes.push_back( std::stoi( cur_bytes, 0, 16 ) );
			i++;
			mask += 'x';
			delete[ ] cur_bytes;
		}
	}

	auto olly_sig = new char[ bytes.size( ) + 1 ];
	std::memcpy( olly_sig, &bytes[ 0 ], bytes.size( ) );
	olly_sig[ bytes.size( ) + 1 ] = '\0';

	auto data = new std::uint8_t[ 25000000 ];
	ReadProcessMemory( handle, ( void* ) engine, &data[ 0 ], 25000000, nullptr );

	for ( std::uint32_t i = 0; i < 25000000; i++ ) {
		if ( cmp( ( std::uint8_t* ) ( &data[ 0 ] + i ), ( std::uint8_t* ) olly_sig, mask.c_str( ) ) ) {
			delete[ ] data;
			delete[ ] olly_sig;
			return engine + i;
		}
	}

	delete[ ] data;
	delete[ ] olly_sig;
	return 0;
}

std::uint32_t get_vfunc( std::uint32_t _interface, int idx ) {
	std::uint32_t vmt = 0;
	ReadProcessMemory( handle, ( void* ) _interface, &vmt, 4, nullptr );
	std::uint32_t vfunc = 0;
	ReadProcessMemory( handle, ( void* ) ( vmt + idx * 4 ), &vfunc, 4, nullptr );

	return vfunc;
}

void hook( void* src, void* dst, int len ) {
	unsigned long prot = 0;
	VirtualProtectEx( handle, src, len, PAGE_EXECUTE_READWRITE, &prot );

	std::uint8_t nop = 0x90;

	for ( auto i = 0; i < len; i++ )
		WriteProcessMemory( handle, ( void* ) ( ( std::uint32_t ) src + i ), &nop, 1, nullptr );

	std::uint8_t jmp_rel32 = 0xE9;
	WriteProcessMemory( handle, src, &jmp_rel32, 1, nullptr );
	std::uint32_t jmp_rel32_val = ( std::uint32_t ) dst - ( std::uint32_t ) src - 5;
	WriteProcessMemory( handle, ( void* ) ( ( std::uint32_t ) src + 1 ), &jmp_rel32_val, 4, nullptr );
	VirtualProtectEx( handle, src, len, prot, &prot );
}

std::uint8_t o_opcodes[ 9 ] { 0 };

std::uint8_t fsn_hk[ ] = {
	/// prologue
	0x55,										// push ebp
	0x89, 0xE5,									// mov ebp, esp

	/// my shellcode
	/// comparing frame stages
	0x83, 0x7D, 0x08, 0x04,						// cmp dword ptr [ ebp + 8 ], FRAME_NET_UPDATE_END
	0x75, 0x4D,									// jne end
	/// putting sounds in "synced" list
	0xB9, 0x00, 0x00, 0x00, 0x00,				// mov ecx, &sound_list_synced
	0xBA, 0x00, 0x00, 0x00, 0x00,				// mov edx, &sound_list
	0x8B, 0x02,									// mov eax, dword ptr [ edx ]
	0x89, 0x01,									// mov dword ptr [ ecx ], eax
	0x8B, 0x42, 0x04,							// mov eax, dword ptr [ edx + 4 ]
	0x89, 0x41, 0x04,							// mov dword ptr [ ecx + 4 ], eax
	0x8B, 0x42, 0x08,							// mov eax, dword ptr [ edx + 8 ]
	0x89, 0x41, 0x08,							// mov dword ptr [ ecx + 8 ], eax
	0x8B, 0x42, 0x0C,							// mov eax, dword ptr [ edx + 12 ]
	0x89, 0x41, 0x0C,							// mov dword ptr [ ecx + 12 ], eax
	0x8B, 0x42, 0x10,							// mov eax, dword ptr [ edx + 16 ]
	0x89, 0x41, 0x10,							// mov dword ptr [ ecx + 16 ], eax
	/// clearing sounds
	0xB9, 0x00, 0x00, 0x00, 0x00,				// mov ecx, &sound_list
	0xC7, 0x01, 0x00, 0x00, 0x00, 0x00,			// mov dword ptr [ ecx ], 0
	0xC7, 0x41, 0x04, 0x00, 0x00, 0x00, 0x00,	// mov dword ptr [ ecx + 4 ], 0
	0xC7, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00,	// mov dword ptr [ ecx + 8 ], 0
	0xC7, 0x41, 0x0C, 0x00, 0x00, 0x00, 0x00,	// mov dword ptr [ ecx + 12 ], 0
	0xC7, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00,	// mov dword ptr [ ecx + 16 ], 0
	/// getting active sounds
	0xB8, 0x00, 0x00, 0x00, 0x00,				// mov eax, S_GetActiveSounds
	0xB9, 0x00, 0x00, 0x00, 0x00,				// mov ecx, &sound_list
	0xFF, 0xD0,									// call eax
	// end:

	/// recreating original instruction
	0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00,			// mov ecx, dword ptr [ VEngineClient ]

	/// jumping back to game's fsn
	0xE9, 0x00, 0x00, 0x00, 0x00				// jmp 0
};

int __stdcall WinMain( HINSTANCE inst, HINSTANCE prev_inst, char* cmd_line, int cmd_show ) {
	/*
	std::atexit( [ ] ( ) {
		// restoring opcodes
		WriteProcessMemory( handle, ( void* ) fsn, &o_opcodes[ 0 ], sizeof( o_opcodes ), nullptr );

		// freeing allocated memory
		VirtualFreeEx( handle, ( void* ) hk, sizeof( fsn_hk ), MEM_RELEASE );
		VirtualFreeEx( handle, ( void* ) sound_list, 65536, MEM_RELEASE );
		VirtualFreeEx( handle, ( void* ) sound_list_synced, 65536, MEM_RELEASE );
	} );
	*/
	
	if ( !FindWindowA( nullptr, "Counter-Strike: Global Offensive" ) )
		Sleep( 500 );

	GetWindowThreadProcessId( FindWindowA( nullptr, "Counter-Strike: Global Offensive" ), &pid );
	handle = OpenProcess( PROCESS_ALL_ACCESS, false, pid );

	client = get_module( "client_panorama.dll" );
	engine = get_module( "engine.dll" );

	vmatrix = find_pattern_client( "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9" ) + 3;
	ReadProcessMemory( handle, ( void* ) vmatrix, &vmatrix, 4, nullptr );
	vmatrix += 176;
	local_player = find_pattern_client( "8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF" ) + 3;
	ReadProcessMemory( handle, ( void* ) local_player, &local_player, 4, nullptr );
	local_player += 4;
	entity_list = find_pattern_client( "BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8" ) + 1;
	ReadProcessMemory( handle, ( void* ) entity_list, &entity_list, 4, nullptr );
	fsn = find_pattern_client( "55 8B EC 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0 A2 ? ? ? ? 8B" );
	s_getactivesounds = find_pattern_engine( "55 8B EC 83 E4 F8 81 EC 44 03 00 00 53 56" );

	ReadProcessMemory( handle, ( void* ) fsn, &o_opcodes[ 0 ], sizeof( o_opcodes ), nullptr );
	
	hk = ( std::uint32_t ) VirtualAllocEx( handle, nullptr, sizeof( fsn_hk ), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
	sound_list = ( std::uint32_t ) VirtualAllocEx( handle, nullptr, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );
	sound_list_synced = ( std::uint32_t ) VirtualAllocEx( handle, nullptr, 65536, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

	// getting original instructions
	auto bytes = 0;
	ReadProcessMemory( handle, ( void* ) ( ( std::uint32_t ) fsn + 5 ), &bytes, 4, nullptr );
	std::memcpy( &fsn_hk[ 100 ], &bytes, 4 );

	// modifying shellcode
	auto jmp_back_rel32 = ( std::uint32_t ) fsn - ( std::uint32_t ) hk - 5 - sizeof( fsn_hk ) + 14;
	std::memcpy( &fsn_hk[ 105 ], &jmp_back_rel32, 4 );

	auto get_active_sounds = ( std::uint32_t ) s_getactivesounds;
	std::memcpy( &fsn_hk[ 87 ], &s_getactivesounds, 4 );

	auto sound_list_ptr = ( std::uint32_t ) sound_list;
	auto sound_list_synced_ptr = ( std::uint32_t ) sound_list_synced;
	std::memcpy( &fsn_hk[ 10 ], &sound_list_synced_ptr, 4 );
	std::memcpy( &fsn_hk[ 15 ], &sound_list_ptr, 4 );
	std::memcpy( &fsn_hk[ 48 ], &sound_list_ptr, 4 );
	std::memcpy( &fsn_hk[ 92 ], &sound_list_ptr, 4 );

	// writing shellcode to memory
	WriteProcessMemory( handle, ( void* ) hk, &fsn_hk[ 0 ], sizeof( fsn_hk ), nullptr );

	// hooking fsn
	hook( ( void* ) fsn, ( void* ) hk, 9 );

	MessageBoxA( nullptr, "Sound list initialized.", "", MB_OK );

	// setting up title and class name
	classname = gen_random_str( 32 );
	wndname = gen_random_str( 32 );

	// creating overlay widnow
	memset( &wndclass, 0, sizeof( WNDCLASSEXA ) );

	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.cbSize = sizeof( WNDCLASSEXA );
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = ( WNDPROC ) wndproc;
	wndclass.hInstance = inst;
	wndclass.hCursor = LoadCursorA( nullptr, IDC_ARROW );
	wndclass.hIcon = LoadIconA( 0, IDI_APPLICATION );
	wndclass.hIconSm = LoadIconA( 0, IDI_APPLICATION );
	wndclass.hbrBackground = ( HBRUSH ) RGB( 0, 0, 0 );
	wndclass.lpszClassName = classname.c_str( );
	wndclass.lpszMenuName = wndname.c_str( );

	RegisterClassExA( &wndclass );

	overlay = CreateWindowExA( WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT, classname.c_str( ), wndname.c_str( ), WS_POPUP | WS_VISIBLE, 0, 0, 1920, 1080, nullptr, nullptr, nullptr, nullptr );

	MARGINS margin = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea( overlay, &margin );

	SetLayeredWindowAttributes( overlay, RGB( 0, 0, 0 ), 255, LWA_COLORKEY );

	_beginthread( ( _beginthread_proc_type ) run_find_window, 0, nullptr );
	_beginthread( ( _beginthread_proc_type ) run_get_sound_data, 0, nullptr );
	
	GetWindowRect( window, &rect );
	MoveWindow( overlay, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true );

	while ( PeekMessageA( &msg, overlay, 0, 0, PM_REMOVE ) ) {
		GetWindowRect( window, &rect );
		MoveWindow( overlay, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true );
		DispatchMessageA( &msg );
		TranslateMessage( &msg );

		Sleep( 1 );
	}
}