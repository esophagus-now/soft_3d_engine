#ifndef MAIN_H
#define MAIN_H 1

#include <iostream>
#include <vector>

#define BUFFER_SPACE 16

#define WIDTH 1900
#define HEIGHT 1000

#define FOV 1.5

#define DEBUG 1

#if (DEBUG == 1)
#define DBG_PRINT(x) cout << #x " = " << x << el
#define DBG_PUTS(x) cout << x << el
#else
#define DBG_PRINT(x)
#define DBG_PUTS(x)
#endif


std::ostream& el(std::ostream &o);

template <typename T>
std::ostream& operator<<(std::ostream &o, std::vector<T> const& v) {
	o << "[";
	auto delim = "";
	for (auto const& i : v) {
		o << delim << i;
		delim = ", ";
	}
	return o << "(" << v.size() << ")]";
}

typedef unsigned char byte;

#pragma pack(1)
struct sdl_pixel {
	byte b, g, r;
	byte unused;
};

#endif


/*
VESTIGIAL TAILS

struct vec {
	float x, y, z;
	
	vec operator* (float f) const {
		return {f*x, f*y, f*z};
	}
	
	vec operator+ (vec const& other) const {
		return {x+other.x, y+other.y, z+other.z};
	}
	
	vec operator- (vec const& other) const {
		return {x-other.x, y-other.y, z-other.z};
	}
	
	float operator* (vec const& other) const {
		return x*other.x + y*other.y + z*other.z;
	}
	
	float mag_sq() const {
		return x*x + y*y + z*z;
	}
	
	float mag() const {
		return sqrtf(mag_sq());
	}
	
	vec& normalize() {
		float m = 1.0f/mag();
		x *= m; y *= m; z *= m;
		return *this;
	}	
};

vec operator* (float f, vec const& v) {
	return v*f;
}














void print_overlap_info(segment const& a, segment const& b) {
	float a_dx = (float) a.xr - (float) a.xl;
	float a_dz = a.zr_inv - a.zl_inv;
	
	//No need to worry about machine epsilon here
	if (a_dx == 0.0) {
		cout << "Existing segment is only one pixel. Requires special case" << el;
		//Left and right must have same z coordinate or else tis doesn't make sense
		return;
	}
	
	float zl_inv_xformed, zr_inv_xformed;
	if (fabsf(a_dz) < 1e-7) {
		cout << "No matrix transformation required, but still need to shift" << el;
		zl_inv_xformed = b.zl_inv - a.zl_inv;
		zr_inv_xformed = b.zr_inv - a.zr_inv;
	} else {
		cout << "Safe to perform transform" << el;
		float M22 = -a_dx/a_dz;
		
		float xl_shifted = (float) (b.xl - a.xl);
		float xr_shifted = (float) (b.xr - a.xr);
		float zl_inv_shifted = b.zl_inv - a.zl_inv;
		float zr_inv_shifted = b.zr_inv - a.zr_inv;
		zl_inv_xformed = xl_shifted + M22*zl_inv_shifted;
		zr_inv_xformed = xr_shifted + M22*zr_inv_shifted;
	}
	
	bool l_behind = (zl_inv_xformed < 0);
	bool r_behind = (zr_inv_xformed < 0);
	
	cout << "Left point is " << (l_behind  ? "behind" : "in front") << el;
	cout << "Right point is " << (r_behind ? "behind" : "in front") << el;
	
	if (l_behind) {
		if (r_behind) {
			if (b.xl < a.xl) {
				cout << "Insert red line chopped from " << b.xl << " to " << a.xl - 1 << el;
				//repl.push_back(chop_right(
			}
			cout << "Insert entire black line (from " << a.xl << " to " << a.xr << ")" << el;
			if (b.xr > a.xr) {
				cout << "Insert red line chopped from" << a.xr+1 << " to " << b.xr << el;
			}
		} else {
			cout << "Not implemented!" << el;
		}
	} else {
		if (r_behind) {
			cout << "Not implemented!" << el;
		} else {
			if (a.xl < b.xl) {
				cout << "Insert black line chopped from " << a.xl << " to " << b.xl - 1 << el;
			}
			cout << "Insert entire red line (from " << b.xl << " to " << b.xr << ")" << el;
			if (a.xr > b.xr) {
				cout << "Insert black line chopped from" << b.xr+1 << " to " << a.xr << el;
			}
		}
	}
	
}


* */
