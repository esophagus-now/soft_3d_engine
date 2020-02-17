#include <iostream>
#include <cmath>

using namespace std; 

ostream& el(ostream &o) {
	return o << "\n";
}

struct segment {
	short xl, xr;
	float zl_inv, zr_inv; //Could make these ints. We know the range of the 
	//view frustum, so we just need a fairly precise (and monotonically 
	//increasing) float->int mapping
	//tri const *t; //Gives us access to gradients
	
	int sect(segment const& other) {
		return 0;
	}
	
};

void print_overlap_info(segment const& a, segment const& b) {
	float a_dx = (float) a.xr - (float) a.xl;
	float a_dz = a.zr_inv - a.zl_inv;
	
	//No need to worry about machine epsilon here
	if (a_dx == 0.0) {
		cout << "Existing segment is only one pixel. Requires special case" << el;
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
	
	cout << "Left point is " << (zl_inv_xformed < 0 ? "behind" : "in front") << el;
	cout << "Right point is " << (zr_inv_xformed < 0 ? "behind" : "in front") << el;
}

int main(int argc, char **argv) {	
	segment s1 = {
		5, 6,
		0.25, 0.25
	};
	
	segment s2 = {
		6, 8,
		0.3, 0.3
	};
	
	print_overlap_info(s1, s2);
	
	return 0;
}
