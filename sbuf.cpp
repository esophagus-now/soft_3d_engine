#include <iostream>
#include <cmath>
#include <vector>
#include "sbuf.h"
#include "main.h"

using namespace std;

//In order to match my drawings, I have named the two arguments to this 
//function the "black" and "red" segments. This chops up the red and black
//segments and pushes all visible segments into r. 
//The return value is a bit difficult to explain. Basically, this only adds
//visible segments with xr <= blk.xr. The reason for this isn't clear until
//you look at sbuffer.insert(segment const& s, int y)
segment add_visible_segs(segment const& blk, segment const& red, float red_gradz, std::vector<segment> &r) {
	//Get dz and dx for black segment
	float blk_dx = (float) (blk.xr - blk.xl);
	float blk_dz = blk.zr_inv - blk.zl_inv;
	
	//Forward-declare the red segment's "transformed" z values. The trick
	//here is that I shift my coordinate system so that the black line is
	//along the x axis, and that the black line's left x value is at the
	//origin. Then, I only need to check the signs of the red segment's
	//transformed z values to see if they're behind or in front of the 
	//black line.
	float zl_inv_xformed, zr_inv_xformed;
	
	//Forward-declare the black segment's dz/dx. Unfortunately, we have to 
	//check for degenerate cases (i.e. dx = 0) and treat them specially 
	//This are pre-computed to speed up chopping calculations
	float blk_gradz;
	
	//No need to worry about machine epsilon for blk_dx
	if (blk_dx == 0.0 || fabsf(blk_dz) < 1e-36) {
		//Slope of z along black segment
		blk_gradz = 0.0;
		
		//No matrix transformation required, but still need to shift
		zl_inv_xformed = red.zl_inv - blk.zl_inv;
		zr_inv_xformed = red.zr_inv - blk.zr_inv;
	} else {
		//Slope of z along black segment
		blk_gradz = blk_dz/blk_dx;
		
		//Safe to perform transform
		float M22 = -blk_dx/blk_dz;
		
		float xl_shifted = (float) (red.xl - blk.xl);
		float xr_shifted = (float) (red.xr - blk.xr);
		float zl_inv_shifted = red.zl_inv - blk.zl_inv;
		float zr_inv_shifted = red.zr_inv - blk.zr_inv;
		zl_inv_xformed = xl_shifted + M22*zl_inv_shifted;
		zr_inv_xformed = xr_shifted + M22*zr_inv_shifted;
	}
	
	bool l_behind = (zl_inv_xformed < 0);
	bool r_behind = (zr_inv_xformed < 0);
	
	if (l_behind) {
		if (r_behind) {
			if (red.xl < blk.xl) {
				//Insert red line chopped from red.xl to (blk.xl-1)
				r.push_back(red.chop_right(blk.xl-1, red_gradz));
			}
			//Insert entire black line (from blk.xl to blk.xr)
			r.push_back(std::move(blk));
			//Return what is left of the red segment
			return red.chop_left(blk.xr+1, red_gradz);
		} else {
			std::cout << "Not implemented!" << el;
			return red;
		}
	} else {
		if (r_behind) {
			std::cout << "Not implemented!" << el;
			return red;
		} else {
			if (blk.xl < red.xl) {
				//Insert black line chopped from blk.xl to (red.xl-1)
				r.push_back(blk.chop_right(red.xl-1, blk_gradz));
			}
			if (blk.xr > red.xr) {
				//Special case: the red segment is fully inserted 
				r.push_back(red);
				//Insert black line chopped from (red.xr+1) to blk.xr
				r.push_back(blk.chop_left(red.xr+1, blk_gradz));
			}
			//Return what is left of the red segment
			return red;
		}
	}
}

ostream& operator<< (ostream &o, segment const& s) {
	return o << "<" << s.xl << "," << s.xr << ">";
}
