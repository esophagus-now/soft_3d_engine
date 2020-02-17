#ifndef SBUF_H
#define SBUF_H 1


struct segment {
	short xl, xr;
	float zl_inv, zr_inv; //Could make these ints. We know the range of the 
	//view frustum, so we just need a fairly precise (and monotonically 
	//increasing) float->int mapping
	tri const *t; //Gives us access to gradients
		
	//Goes from xl----------------xr 
	//       to        new_left---xr
	//In the function that calls this one, we will have already calculated
	//dz_inv / dx 
	//ASSUMPTION: new_left is strictly less than xr
	segment chop_left(short new_left, float dzinv_dx) {
#if DEBUG == 1
		assert(new_left < xr);
#endif
		float new_z_inv = zl_inv + dzinv_dx*(float)(new_left - xl);
		return {new_left, xr, new_z_inv, zr_inv, t};
	}
	//But we still provide an overload if we don't have dzinv_dx
	segment chop_left(short new_left) {
		float dz_inv = zr_inv - zl_inv;
		
		float dzinv_dx; 
		if (xr != xl) dzinv_dx = dz_inv / (xr - xl);
		else dzinv_dx = 0.0;
		
		return chop_left(new_left, dzinv_dx);
	}
	
	
	//ASSUMPTION: new_right is strictly greater than xl
	segment chop_right(short new_right, float dzinv_dx) {
#if DEBUG == 1
		assert(new_right > xl);
#endif
		float new_z_inv = zl_inv + dzinv_dx*(float)(new_right - xl);
		return {xl, new_right, zl_inv, new_z_inv, t};
	}
	
	segment chop_right(short new_left) {
		float dz_inv = zr_inv - zl_inv;
		
		float dzinv_dx; 
		if (xr != xl) dzinv_dx = dz_inv / (xr - xl);
		else dzinv_dx = 0.0;
		
		return chop_left(new_left, dzinv_dx);
	}
};

template <int N_SCANLINES>
struct sbuffer {
	std::vector<segment> scanlines[N_SCANLINES];
	
	void insert(segment const& s, int y) {
		std::vector<segment> const& orig = scanlines[y];
		std::vector<segment> repl; //Replacement scanline
		repl.reserve(orig.size() + 16); //Should prevent reallocations as we work
		
		//Immediately copy all segments to the left of the "contested area"
		unsigned i;
		for (i = 0; orig[i].xr < s.xl; i++) {
			repl.push_back(orig[i]);
		}
		
		//... do middle section ...
		for (; orig[i].xl <= s.xr; i++) {
			
		}
		
		//Finally, copy in everything to the rigtht of the "contested area"
		for (; i < orig.size(); i++) {
			repl.push_back(orig[i]);
		}
	}
};


#endif
