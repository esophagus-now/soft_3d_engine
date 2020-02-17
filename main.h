#ifndef MAIN_H
#define MAIN_H 1

#include <iostream>
#include <vector>

#define BUFFER_SPACE 16

#define WIDTH 400
#define HEIGHT 400

#define DEBUG 0

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



* */
