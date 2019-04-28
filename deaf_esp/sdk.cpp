#include <Windows.h>
#include "sdk.h"

extern RECT rect;

bool world_to_screen( vec_t in, vec_t& out, vmatrix_t matrix ) {
	out.x = matrix[ 0 ][ 0 ] * in.x + matrix[ 0 ][ 1 ] * in.y + matrix[ 0 ][ 2 ] * in.z + matrix[ 0 ][ 3 ];
	out.y = matrix[ 1 ][ 0 ] * in.x + matrix[ 1 ][ 1 ] * in.y + matrix[ 1 ][ 2 ] * in.z + matrix[ 1 ][ 3 ];

	auto w = matrix[ 3 ][ 0 ] * in[ 0 ] + matrix[ 3 ][ 1 ] * in[ 1 ] + matrix[ 3 ][ 2 ] * in[ 2 ] + matrix[ 3 ][ 3 ];

	if ( w < 0.001f )
		return false;

	out.x /= w;
	out.y /= w;

	auto x = ( rect.right - rect.left ) / 2.0f;
	auto y = ( rect.bottom - rect.top ) / 2.0f;

	x += 0.5f * out.x * ( rect.right - rect.left ) + 0.5f;
	y -= 0.5f * out.y * ( rect.bottom - rect.top ) + 0.5f;

	out.x = x;
	out.y = y;

	return true;
}