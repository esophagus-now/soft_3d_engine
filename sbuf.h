#ifndef SBUF_H
#define SBUF_H 1

#include <SDL.h>
#include <iostream>
#include "main.h"
#include "3d_types.h"
#include "poly_raster.h"

struct segment {
	short xl, xr;
	float zl_inv, zr_inv; //Could make these ints. We know the range of the 
	//view frustum, so we just need a fairly precise (and monotonically 
	//increasing) float->int mapping
	tri const *t; //Gives us access to gradients
		
	//Goes from:      xl----------------xr 
	//       to:             new_left---xr
	//In the function that calls this one, we will have already calculated
	//dz/dx, but an overload is provided below if the caller doesn't have it
	segment chop_left(short new_left, float dzinv_dx) const {
		float new_z_inv = zl_inv + dzinv_dx*(float)(new_left - xl);
		return {new_left, xr, new_z_inv, zr_inv, t};
	}
	//General chop_left
	segment chop_left(short new_left) const {
		return chop_left(new_left, this->gradz());
	}
	
	//Same as chop_left, but on the right
	segment chop_right(short new_right, float dzinv_dx) const {
		float new_z_inv = zl_inv + dzinv_dx*(float)(new_right - xl);
		return {xl, new_right, zl_inv, new_z_inv, t};
	}
	//General chop_right
	segment chop_right(short new_right) const {
		return chop_right(new_right, this->gradz());
	}
	
	//Get segment's dz/dx
	float gradz() const {		
		float ret;
		
		float dx = (float) (xr - xl);
		float dz = zr_inv - zl_inv;
		if (dx == 0.0) {
			//This (should) only happen when the red segment is a single pixel.
			//In that case, it is sensible to say dz/dx is 0
			ret = 0.0;
		} else {
			ret = dz/dx;
		}
		
		return ret;
	}
};

std::ostream& operator<< (std::ostream &o, segment const& s);

//In order to match my drawings, I have named the two arguments to this 
//function the "black" and "red" segments. This chops up the red and black
//segments and pushes all visible segments into r. 
//The return value is a bit difficult to explain. Basically, this only adds
//visible segments with xr <= blk.xr. The reason for this isn't clear until
//you look at sbuffer.insert(segment const& s, int y)
segment add_visible_segs(segment const& blk, segment const& red, float red_gradz, std::vector<segment> &r);

template <int N_SCANLINES>
struct sbuffer {
	std::vector<segment> scanlines[N_SCANLINES];
	
	void insert_segment(segment s, int y) {
		
		//Discard segments that are out of range
		if (y < 0 || y >= N_SCANLINES) return;
		if (s.xr < 0 || s.xl >= WIDTH) return;
		
		//Clip segment to screen
		if (s.xl < 0) {
			s = s.chop_left(0);
		}
		if (s.xr >= WIDTH) {
			s = s.chop_right(WIDTH - 1);
		}
		
		std::vector<segment> const& orig = scanlines[y];
		std::vector<segment> repl; //Replacement scanline
		repl.reserve(orig.size() + BUFFER_SPACE); //Should prevent reallocations as we work
		
		
		//Immediately copy all segments to the left of the "contested area"
		unsigned i;
		for (i = 0; i < orig.size(); i++) {
			if (orig[i].xr >= s.xl) break;
			repl.push_back(orig[i]);
		}
		
		//... do middle section ...
		float gradz = s.gradz();
		segment cpy(s);
		for (; i < orig.size(); i++) {
			if (orig[i].xl > s.xr) break;
			cpy = add_visible_segs(orig[i], cpy, gradz, repl);
		}
		
		//Add rest of new segment if needed
		if (cpy.xl <= cpy.xr) {
			repl.push_back(cpy);
		}
		
		//Finally, copy in everything to the rigtht of the "contested area"
		for (; i < orig.size(); i++) {
			repl.push_back(orig[i]);
		}
		
		scanlines[y] = std::move(repl);
	}
	
	void insert_tri(tri const& t, vert_shaded const* v) {
		std::vector<int> lefts, rights;
		int y;
		scan_tri(lefts, rights, y, v, t);
		for (unsigned i = 0; i < lefts.size(); i++, y++) {
			insert_segment({(short)lefts[i], (short)rights[i], 0.0, 0.0, &t}, y);
		}
	}
	
	//Assume N is equal to (or less than) the height of s
	void draw(SDL_Surface *s) {
		for (int i = 0; i < HEIGHT; i++) {
			sdl_pixel *line = (sdl_pixel *) ((char*)(s->pixels) + i*s->pitch);
			std::vector<segment> const& segs = scanlines[i];
			for (auto const& seg : segs) {
				for (int j = seg.xl; j <= seg.xr; j++) {
					line[j].r = seg.t->r;
					line[j].g = seg.t->g;
					line[j].b = seg.t->b;
				}
			}
		}
	}
	
	void clear() {
		for (auto &v : scanlines) v.clear();
	}
};

template <int deriveme>
std::ostream& operator<< (std::ostream &o, sbuffer<deriveme> const& sbuf) {
	o << "sbuffer:";
	auto y_delim = "\n\t";
	for (int i = 0; i < deriveme; i++) {
		auto const& v = sbuf.scanlines[i];
		if (v.size() > 0) {
			o << y_delim << "y = " << i << ": {";
			auto delim = "";
			for (auto const& seg : v) {
				o << delim << seg;
				delim = " | ";
			}
			o << "}";
		}
	}
	
	return o;
}


#endif
