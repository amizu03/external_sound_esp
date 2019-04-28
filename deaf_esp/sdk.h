#pragma once
#include <cmath>
#include <limits>
#include <cstdint>

class vec_t {
public:
	float x, y, z;

	vec_t( void ) {
		init( );
	}

	vec_t( float _x, float _y, float _z = 0.0f ) {
		x = _x;
		y = _y;
		z = _z;
	}

	void init( void ) {
		x = y = z = 0.0f;
	}

	void init( float _x, float _y, float _z ) {
		x = _x;
		y = _y;
		z = _z;
	}

	bool is_valid( void ) {
		return std::isfinite( x ) && std::isfinite( y ) && std::isfinite( z );
	}

	void invalidate( void ) {
		x = y = z = std::numeric_limits< float >::infinity( );
	}

	void clear( void ) {
		x = y = z = 0;
	}

	float& operator[]( int i )
	{
		return ( ( float* ) this )[ i ];
	}

	float operator[]( int i ) const
	{
		return ( ( float* ) this )[ i ];
	}

	void zero( void ) {
		x = y = z = 0.0f;
	}

	bool operator==( const vec_t& src ) const {
		return ( src.x == x ) && ( src.y == y ) && ( src.z == z );
	}

	bool operator!=( const vec_t& src ) const {
		return ( src.x != x ) || ( src.y != y ) || ( src.z != z );
	}

	vec_t& operator+=( const vec_t& v ) {
		x += v.x; y += v.y; z += v.z;
		return *this;
	}

	vec_t& operator-=( const vec_t& v ) {
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}

	vec_t& operator*=( float fl ) {
		x *= fl;
		y *= fl;
		z *= fl;
		return *this;
	}

	vec_t& operator*=( const vec_t& v ) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	vec_t& operator/=( const vec_t& v ) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	vec_t& operator+=( float fl ) {
		x += fl;
		y += fl;
		z += fl;
		return *this;
	}

	vec_t& operator/=( float fl ) {
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}

	vec_t& operator-=( float fl ) {
		x -= fl;
		y -= fl;
		z -= fl;
		return *this;
	}

	void normalize( ) {
		*this = normalized( );
	}

	vec_t normalized( ) const {
		vec_t res = *this;
		float l = res.length( );

		if ( l != 0.0f )
			res /= l;
		else
			res.x = res.y = res.z = 0.0f;

		return res;
	}

	float dist_to( const vec_t& vec ) const {
		vec_t delta;

		delta.x = x - vec.x;
		delta.y = y - vec.y;
		delta.z = z - vec.z;

		return delta.length( );
	}

	float dist_to_sqr( const vec_t& vec ) const {
		vec_t delta;

		delta.x = x - vec.x;
		delta.y = y - vec.y;
		delta.z = z - vec.z;

		return delta.length_sqr( );
	}

	float dot_product( const vec_t& vec ) const {
		return ( x * vec.x + y * vec.y + z * vec.z );
	}

	vec_t cross_product( const vec_t& vec ) const {
		return vec_t( y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x );
	}

	float length( ) const {
		return sqrt( x*x + y * y + z * z );
	}

	float length_sqr( void ) const {
		return ( x*x + y * y + z * z );
	}

	float length_2d( ) const {
		return sqrt( x * x + y * y );
	}

	vec_t& operator=( const vec_t& vec ) {
		x = vec.x; y = vec.y; z = vec.z;
		return *this;
	}

	vec_t operator-( void ) const {
		return vec_t( -x, -y, -z );
	}

	vec_t operator+( const vec_t& v ) const {
		return vec_t( x + v.x, y + v.y, z + v.z );
	}

	vec_t operator-( const vec_t& v ) const {
		return vec_t( x - v.x, y - v.y, z - v.z );
	}

	vec_t operator*( float fl ) const {
		return vec_t( x * fl, y * fl, z * fl );
	}

	vec_t operator*( const vec_t& v ) const {
		return vec_t( x * v.x, y * v.y, z * v.z );
	}

	vec_t operator/( float fl ) const {
		return vec_t( x / fl, y / fl, z / fl );
	}

	vec_t operator/( const vec_t& v ) const {
		return vec_t( x / v.x, y / v.y, z / v.z );
	}
};

__forceinline vec_t operator*( float lhs, const vec_t& rhs ) {
	return rhs * lhs;
}

__forceinline vec_t operator/( float lhs, const vec_t& rhs ) {
	return rhs / lhs;
}

class vmatrix_t {
public:
	float values[ 4 ][ 4 ];

	float* operator[]( int i ) {
		return values[ i ];
	}

	const float* operator[]( int i ) const {
		return values[ i ];
	}
};

class matrix3x4_t {
public:
	float values[ 3 ][ 4 ];

	matrix3x4_t( void ) {
		values[ 0 ][ 0 ] = 0.0f; values[ 0 ][ 1 ] = 0.0f; values[ 0 ][ 2 ] = 0.0f; values[ 0 ][ 3 ] = 0.0f;
		values[ 1 ][ 0 ] = 0.0f; values[ 1 ][ 1 ] = 0.0f; values[ 1 ][ 2 ] = 0.0f; values[ 1 ][ 3 ] = 0.0f;
		values[ 2 ][ 0 ] = 0.0f; values[ 2 ][ 1 ] = 0.0f; values[ 2 ][ 2 ] = 0.0f; values[ 2 ][ 3 ] = 0.0f;
	}

	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23 ) {
		values[ 0 ][ 0 ] = m00; values[ 0 ][ 1 ] = m01; values[ 0 ][ 2 ] = m02; values[ 0 ][ 3 ] = m03;
		values[ 1 ][ 0 ] = m10; values[ 1 ][ 1 ] = m11; values[ 1 ][ 2 ] = m12; values[ 1 ][ 3 ] = m13;
		values[ 2 ][ 0 ] = m20; values[ 2 ][ 1 ] = m21; values[ 2 ][ 2 ] = m22; values[ 2 ][ 3 ] = m23;
	}

	void init( const vec_t& x, const vec_t& y, const vec_t& z, const vec_t &origin ) {
		values[ 0 ][ 0 ] = x.x; values[ 0 ][ 1 ] = y.x; values[ 0 ][ 2 ] = z.x; values[ 0 ][ 3 ] = origin.x;
		values[ 1 ][ 0 ] = x.y; values[ 1 ][ 1 ] = y.y; values[ 1 ][ 2 ] = z.y; values[ 1 ][ 3 ] = origin.y;
		values[ 2 ][ 0 ] = x.z; values[ 2 ][ 1 ] = y.z; values[ 2 ][ 2 ] = z.z; values[ 2 ][ 3 ] = origin.z;
	}

	matrix3x4_t( const vec_t& x, const vec_t& y, const vec_t& z, const vec_t &origin ) {
		init( x, y, z, origin );
	}

	inline void set_origin( vec_t const& p ) {
		values[ 0 ][ 3 ] = p.x;
		values[ 1 ][ 3 ] = p.y;
		values[ 2 ][ 3 ] = p.z;
	}

	inline void invalidate( void ) {
		for ( int i = 0; i < 3; i++ ) {
			for ( int j = 0; j < 4; j++ ) {
				values[ i ][ j ] = std::numeric_limits<float>::infinity( );;
			}
		}
	}

	vec_t get_x_axis( void ) const {
		return at( 0 );
	}

	vec_t get_y_axis( void ) const {
		return at( 1 );
	}

	vec_t get_z_axis( void ) const {
		return at( 2 );
	}

	vec_t get_origin( void ) const {
		return at( 3 );
	}

	vec_t at( int i ) const {
		return vec_t { values[ 0 ][ i ], values[ 1 ][ i ], values[ 2 ][ i ] };
	}

	float* operator[]( int i ) {
		return values[ i ];
	}

	const float* operator[]( int i ) const {
		return values[ i ];
	}

	float* base( ) {
		return &values[ 0 ][ 0 ];
	}

	const float* base( ) const {
		return &values[ 0 ][ 0 ];
	}

	const bool operator==( matrix3x4_t matrix ) const {
		return
			values[ 0 ][ 0 ] == matrix[ 0 ][ 0 ] && values[ 0 ][ 1 ] == matrix[ 0 ][ 1 ] && values[ 0 ][ 2 ] == matrix[ 0 ][ 2 ] && values[ 0 ][ 3 ] == matrix[ 0 ][ 3 ] &&
			values[ 1 ][ 0 ] == matrix[ 1 ][ 0 ] && values[ 1 ][ 1 ] == matrix[ 1 ][ 1 ] && values[ 1 ][ 2 ] == matrix[ 1 ][ 2 ] && values[ 1 ][ 3 ] == matrix[ 1 ][ 3 ] &&
			values[ 2 ][ 0 ] == matrix[ 2 ][ 0 ] && values[ 2 ][ 1 ] == matrix[ 2 ][ 1 ] && values[ 2 ][ 2 ] == matrix[ 2 ][ 2 ] && values[ 2 ][ 3 ] == matrix[ 2 ][ 3 ];
	}
};

struct snd_info_t {
	int guid;
	std::uint32_t h_filename;
	int sound_src;
	int channel;
	int speaker_ent;
	float vol;
	float last_spatialized_vol;
	float rad;
	int pitch;
	std::uint32_t p_origin;
	std::uint32_t p_dir;
	bool update_positions;
	bool is_sentence;
	bool dry_mix;
	bool speaker;
	bool from_server;
};

extern bool world_to_screen( vec_t in, vec_t& out, vmatrix_t matrix );