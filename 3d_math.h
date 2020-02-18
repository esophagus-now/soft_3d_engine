#ifndef THREE_D_MATH_H
#define THREE_D_MATH_H 1

#include "3d_types.h"
#include <cmath>
#include <vector>
#include <memory> //For unique_ptr


//Matrix data format is a 1d array in row-major order. However, because
//the bottom row of a transformation matrix is always [0,0,0,1], there's
//no need to include it in the array. The functions in this file are
//specifically optimized for this assumption
struct xform {
	float c[12];
	
	//Initialize to the identity matrix
	xform() : c{0,0,0,0, 0,0,0,0, 0,0,0,0} {}
	
	//Special matrix multiplication
	xform operator*(xform const& other) const {
		xform ret;
	
		ret[0] = c[0]*other.c[0] + c[1]*other.c[4] + c[2]*other.c[8];
		ret[1] = c[0]*other.c[1] + c[1]*other.c[5] + c[2]*other.c[9];
		ret[2] = c[0]*other.c[2] + c[1]*other.c[6] + c[2]*other.c[10];
		ret[3] = c[0]*other.c[3] + c[1]*other.c[7] + c[2]*other.c[11] + c[3];
		
		ret[4] = c[4]*other.c[0] + c[5]*other.c[4] + c[6]*other.c[8];
		ret[5] = c[4]*other.c[1] + c[5]*other.c[5] + c[6]*other.c[9];
		ret[6] = c[4]*other.c[2] + c[5]*other.c[6] + c[6]*other.c[10];
		ret[7] = c[4]*other.c[3] + c[5]*other.c[7] + c[6]*other.c[11] + c[7];
		
		ret[8]  = c[8]*other.c[0] + c[9]*other.c[4] + c[10]*other.c[8];
		ret[9]  = c[8]*other.c[1] + c[9]*other.c[5] + c[10]*other.c[9];
		ret[10] = c[8]*other.c[2] + c[9]*other.c[6] + c[10]*other.c[10];
		ret[11] = c[8]*other.c[3] + c[9]*other.c[7] + c[10]*other.c[11] + c[11];
		
		return ret;
	}
	
	xform& operator*=(xform const& other) {
		return *this = *this * other;
	}
	
	//Transform a single 3D point
	pnt operator*(pnt const& p) const {
		pnt ret;
	
		ret[0] = c[0]*p[0] + c[1]*p[1] + c[2]*p[2] + c[3];
		ret[1] = c[4]*p[0] + c[5]*p[1] + c[6]*p[2] + c[7];
		ret[2] = c[8]*p[0] + c[9]*p[1] + c[10]*p[2] + c[11];
		
		return ret;
	}
	
	float& operator[] (int i) {
		return c[i];
	}
	
	float const& operator[] (int i) const {
		return c[i];
	}
};

struct xformable {
	virtual operator xform() const = 0;
	virtual ~xformable() {} //Why is this so complicated?
};

struct ident : xformable {
	operator xform() const {
		xform ret;
		ret[0] = 1.0;
		ret[5] = 1.0;
		ret[10] = 1.0;
		return ret;
	}
};

struct rot : xformable {
	float ang; //In radians
	
	rot() : ang(0.0) {}
	
	rot(float ang_rad) : ang(ang_rad) {}
	
	float get_rad() const {
		return ang;
	}
	
	void set_rad(float ang_rad) {
		ang = ang_rad;
	}
	
	float get_deg() const {
		return ang*180.0/M_PI;
	}
	
	void set_deg(float ang_deg) {
		ang = ang_deg * M_PI / 180.0;
	}
};

struct xrot : rot {
	xrot(float ang_rad) : rot(ang_rad) {}
	
	virtual operator xform() const {
		/*var rot_x_mat = [
            1      , 0                 , 0                 , 0,
            0      , cos(this.rot[0])  , -sin(this.rot[0]) , 0,
            0      , sin(this.rot[0])  , cos(this.rot[0])  , 0
        ];*/
        
        xform ret;
        ret[0] = 1;
        ret[5] = cosf(ang);
        ret[6] = -sinf(ang);
        ret[9] = sinf(ang);
        ret[10] = cosf(ang);
        
        return ret;
	}
};

struct yrot : rot {
	yrot(float ang_rad) : rot(ang_rad) {}
	
	virtual operator xform() const {
		/*var rot_y_mat = [
            cos(this.rot[1])   , 0 , sin(this.rot[1])  , 0,
            0                  , 1 , 0                 , 0,
            -sin(this.rot[1])  , 0 , cos(this.rot[1])  , 0
        ];*/
        
        xform ret;
        ret[0] = cosf(ang);
        ret[2] = sinf(ang);
        ret[5] = 1.0;
        ret[8] = -sinf(ang);
        ret[10] = cosf(ang);
        
        return ret;
	}
};

struct zrot : rot {
	zrot(float ang_rad) : rot(ang_rad) {}
	
	virtual operator xform() const {
		/*var rot_z_mat = [
            cos(this.rot[2])   , sin(this.rot[2])  , 0 ,   0,
            -sin(this.rot[2])  , cos(this.rot[2])  , 0 ,   0,
            0                  , 0                 , 1 ,   0
        ];*/
        
        xform ret;
        ret[0] = cosf(ang);
        ret[1] = sinf(ang);
        ret[4] = -sinf(ang);
        ret[5] = cosf(ang);
        ret[10] = 1.0;
        
        return ret;
	}
};

struct xlate : xformable {
	float x, y, z;
	virtual operator xform() const {
		xform ret = ident();
		ret[3] = x;
		ret[7] = y;
		ret[11] = z;
		return ret;
	}
};

struct scale : xformable {
	float x, y, z;
	
	scale(float x, float y, float z) : x(x), y(y), z(z) {}
	
	virtual operator xform() const {
		xform ret;
		ret[0] = x;
		ret[5] = y;
		ret[10] = z;
		return ret;
	}
};

void shade_vert_arr(vert_shaded *dest, vert const *src, xform const& mv, int len);

xform get_xform(std::vector<xformable const *> const& v);

#endif
