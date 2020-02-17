#include "main.h"
#include "3d_types.h"
#include <vector>

//Better to pre-allocate and then use array subscripting
void scan_edge(int x0, int y0, int x1, int y1, int lr, int skip_first, std::vector<int> &out) {
	int dx, sx;
	int dy, sy;
	
	//Get abs(x1 - x0) and also set sx to be sign(x1-x0)
	if (x0 < x1) {
		dx = x1 - x0;
		sx = 1;
	} else {
		dx = x0 - x1;
		sx = -1;
	}
	
	//Get abs(y1-y0)
	if (y0 < y1) {
		dy = y1 - y0;
		sy = 1;
	} else {
		dy = y0 - y1;
		sy = -1;
	}
	
	//The only horizontal lines that get into this function are bottom 
	//lines, and the rule is not to draw those. Also, this could happen
	//if two vertices end up on top of one another; these should also be
	//discarded
	if (dy == 0) {
		return;
	}
	
	//Keep track of integer and fractional part of x coordinate as we step y
	int I = x0 + lr; //Integer part of x coordinate
	int N = 0; //Numerator part of fraction of x coordinate
	
	int N_step = dx % dy;
	int I_step = sx*(dx/dy); //Use integer division to compute floor(dx/dy)
	
	int D = dy; //Denominator part
	
	//Special case: sometimes we will deliberately skip the first point
	int init_y = y0;
	if (skip_first != 0) {
		init_y += sy;
		N += N_step;
		I += I_step;
		if (N >= D) {
			N -= D;
			I += sx;
		}
	}
	
	//Finally, scan out all the x values on this edge
	for (int y = init_y; y != y1; y+=sy) {
		//console.log("left: " + I + ", " + y);
		out.push_back(I);
		N += N_step;
		I += I_step;
		if (N >= D) {
			N -= D;
			I += sx;
		}
	}
}

//Better to pre-allocate and then use array subscripting
//Try limiting calls to scan_edge
void scan_tri(std::vector<int> &lefts, std::vector<int> &rights, int& top_y, vert_shaded const *V, tri const& t, int verbose = 0) {
	vert_shaded const& v1 = V[t.ind[0]];
	vert_shaded const& v2 = V[t.ind[1]];
	vert_shaded const& v3 = V[t.ind[2]];
	
	//Find out if triangle faces away (i.e. vertices CCW) , and return in
	//that case
	//This calculates sign of z component of the cross product. If negative,
	//triangle points away and should not be drawn
	int v21x = v2.x - v1.x, v21y = v2.y - v1.y;
	int v31x = v3.x - v1.x, v31y = v3.y - v1.y;
	if (v21x*v31y - v21y*v31x <= 0) return;
	
	//At this point, it is guaranteed that the triangle has nonzero height
	//and width (note that we used <= in the last condition) AND its 
	//vertices are in clockwise order
	
	
	//Find top and bottom vertices and fill vector of x coords	
	int const& a = v1.y;
	int const& b = v2.y;
	int const& c = v3.y;
	
	if (b < a) {
		if (c < b) {
			//c < b <a
			//C top, A bottom
			if (verbose) std::cout << "Case 1" << el;
			top_y = v3.y + 1;
			scan_edge(v3.x, v3.y, v2.x, v2.y, 1, 1, lefts);
			scan_edge(v2.x, v2.y, v1.x, v1.y, 1, 0, lefts);
			scan_edge(v3.x, v3.y, v1.x, v1.y, 0, 1, rights);
		} else if (c < a) {
			//b < a, b<= c, c < a
			//b <= c < a
			//B top, A bottom, check pointiness
			if (verbose) std::cout << "Case 2" << el;
			int top_is_pointy = (v2.y != v3.y);
			top_y = v2.y + top_is_pointy;
			scan_edge(v2.x, v2.y, v1.x, v1.y, 1, top_is_pointy, lefts);
			scan_edge(v2.x, v2.y, v3.x, v3.y, 0, top_is_pointy, rights);
			scan_edge(v3.x, v3.y, v1.x, v1.y, 0, 0, rights);
		} else {
			//b < a, b <= c, a <= c
			//b < a <= c
			//B top, A bottom
			if (verbose) std::cout << "Case 3" << el;
			top_y = v2.y + 1;
			scan_edge(v2.x, v2.y, v1.x, v1.y, 1, 1, lefts);
			scan_edge(v1.x, v1.y, v3.x, v3.y, 1, 0, lefts);
			scan_edge(v2.x, v2.y, v3.x, v3.y, 0, 1, rights);
		}
	} else {
		if (c < a) {
			//a <= b, c < a
			//c < a <= b
			//C top, B bottom
			if (verbose) std::cout << "Case 4" << el;
			top_y = v3.y + 1;
			scan_edge(v3.x, v3.y, v2.x, v2.y, 1, 1, lefts);
			scan_edge(v3.x, v3.y, v1.x, v1.y, 0, 1, rights);
			scan_edge(v1.x, v1.y, v2.x, v2.y, 0, 0, rights);
		} else if (c < b) {
			//a <= b, a <= c, c < b
			//a <= c < b
			//A top, B bottom, check pointiness
			if (verbose) std::cout << "Case 5" << el;
			int top_is_pointy = (v3.y != v1.y);
			top_y = v1.y + top_is_pointy;
			scan_edge(v1.x, v1.y, v3.x, v3.y, 1, top_is_pointy, lefts);
			scan_edge(v3.x, v3.y, v2.x, v2.y, 1, 0, lefts);
			scan_edge(v1.x, v1.y, v2.x, v2.y, 0, top_is_pointy, rights);
		} else {
			//a <= b, a <= c, b <= c
			//a <= b <= c
			//A top, C bottom, check pointiness
			if (verbose) std::cout << "Case 6" << el;
			int top_is_pointy = (v2.y != v1.y);
			top_y = v1.y + top_is_pointy;
			scan_edge(v1.x, v1.y, v3.x, v3.y, 1, top_is_pointy, lefts);
			scan_edge(v1.x, v1.y, v2.x, v2.y, 0, top_is_pointy, rights);
			scan_edge(v2.x, v2.y, v3.x, v3.y, 0, 0, rights);
		}
	}
	
	//Calculate gradients
	
}
