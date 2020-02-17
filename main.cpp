#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cstddef>
#include <utility> //swap
#include <SDL.h>
#include <unistd.h>
#include "main.h"
#include "3d_types.h"
#include "poly_raster.h"
#include "sbuf.h"

using namespace std; 

std::ostream& el(std::ostream &o) {
	return o << "\n";
}

#pragma pack(1)
struct sdl_pixel {
	byte b, g, r;
	byte unused;
};

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

int main(int argc, char **argv) {	
	vert_shaded testverts[4];
	testverts[0].x = 0; testverts[0].y = 40;
	testverts[1].x = 20; testverts[1].y = 0;
	testverts[2].x = 30; testverts[2].y = 30;
	testverts[3].x = 10; testverts[3].y = 80;
	
	tri t1; t1.ind[0] = 0; t1.ind[1] = 1; t1.ind[2] = 2;
	tri t2; t2.ind[0] = 0; t2.ind[1] = 2; t2.ind[2] = 3;
	
	int rc;
	rc = SDL_Init(SDL_INIT_VIDEO);
	if (rc != 0) {
		cout << "Could not initialize SDL Video: " << SDL_GetError() << el;
		exit(-1);
	}
	atexit(SDL_Quit);
	
	auto win = SDL_CreateWindow(
		"Mrs. Fenster", 
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WIDTH, HEIGHT,
		0
	);
	if (!win) {
		cout << "Could not create SDL Window: " << SDL_GetError() << el;
		exit(-1);
	}
	
	int fnum = 0;
	auto surf = SDL_GetWindowSurface(win);
	if (!surf) {
		cout << "Could not create SDL Window Surface: " << SDL_GetError() << el;
		SDL_DestroyWindow(win);
		exit(-1);
	}
	
	unsigned lastTime = SDL_GetTicks();
	unsigned accum_error = 0;
	byte g = 200;
	byte b = 200;
	tri *draw_first = &t1, *draw_second = &t2;
	int edit_index = 0;
	while (1) {
		unsigned thisTime = SDL_GetTicks();
		unsigned delta = thisTime - lastTime;
		lastTime = thisTime;
		
		accum_error += delta;
		
		if (accum_error > 33) {
			//Fill the surface with a colour, later replace this with 3D drawing code
			for (int i = 0; i < HEIGHT; i++) {
				sdl_pixel *line = (sdl_pixel *) ((char*)(surf->pixels) + i*surf->pitch);
				for (int j = 0; j < WIDTH; j++) {
					line[j].r = (255 * fnum) / 30;
					line[j].g = g;
					line[j].b = b;
				}
			}
			
			//Draw triangle!!
			vector<int> lefts, rights;
			int top_y;
			scan_tri(lefts, rights, top_y, testverts, *draw_first);
			
			for (int i = 0, y = top_y; i < int(lefts.size()); i++, y++) {
				sdl_pixel *line = (sdl_pixel *) ((char*)(surf->pixels) + y*surf->pitch);
				for (int x = lefts[i]; x <= rights[i]; x++) {
					line[x].r = (draw_first == &t2)*255;
					line[x].g = 0;
					line[x].b = 0;
				}
			}
			
			lefts.clear(); rights.clear();
			scan_tri(lefts, rights, top_y, testverts, *draw_second);
			
			for (int i = 0, y = top_y; i < int(lefts.size()); i++, y++) {
				sdl_pixel *line = (sdl_pixel *) ((char*)(surf->pixels) + y*surf->pitch);
				for (int x = lefts[i]; x <= rights[i]; x++) {
					line[x].r = (draw_second == &t2)*255;
					line[x].g = 0;
					line[x].b = 0;
				}
			}
			swap(draw_first, draw_second);
			SDL_UpdateWindowSurface(win);
			
			unsigned numFrames = accum_error/33;
			accum_error %= 33;
			fnum = (fnum + numFrames) % 33;
			
			if (numFrames > 1) {
				cout << "Frame dropped" << el;
			}
		}
		
		SDL_Event e;
		int canary = 0;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_WINDOWEVENT) {
				if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
					cout << "Goodbye!" << el;
					canary = 1;
				}
			} else if (e.type == SDL_QUIT) {
				cout << "Program quit after " << e.quit.timestamp << " ticks" << el;
				canary = 1;
			} else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_UP:
					if (testverts[edit_index].y > 0) testverts[edit_index].y--;
					break;
				case SDLK_DOWN:
					if (testverts[edit_index].y < HEIGHT-1) testverts[edit_index].y++;
					break;
				case SDLK_LEFT:
					if (testverts[edit_index].x > 0) testverts[edit_index].x--;
					break;
				case SDLK_RIGHT:
					if (testverts[edit_index].x < WIDTH-1) testverts[edit_index].x++;
					break;
				case SDLK_SPACE:
					edit_index = (edit_index + 1) % 4;
					break;
				case SDLK_RETURN: {
					cout << vector<vert_shaded>(testverts, testverts + 4) << el;
					vector<int> lefts, rights;
					int top_y;
					scan_tri(lefts, rights, top_y, testverts, t1, 1);
					cout << "Black triangle lefts: " << lefts << el;
					cout << "Black triangle rights: " << rights << el;
					cout << "Black triangle top_y: " << top_y << el;
					lefts.clear();
					rights.clear();
					scan_tri(lefts, rights, top_y, testverts, t2, 1);
					cout << "Red triangle lefts: " << lefts << el;
					cout << "Red triangle rights: " << rights << el;
					cout << "Red triangle top_y: " << top_y << el;
					break;
				}
				}
			}
		}
		if (canary) break;
		
		//Remeasure time now that we might have handled events
		thisTime = SDL_GetTicks();
		delta = thisTime - lastTime;
		lastTime = thisTime;
		accum_error += delta;
		
		//A little backwards, but whatever
		if (accum_error < 33) {
			if (SDL_WaitEventTimeout(&e, 33 - accum_error))
				SDL_PushEvent(&e);
		}
	}
	
	SDL_DestroyWindow(win);
	
	segment s1 = {
		3, 18,
		0.5, 0.25
	};
	
	segment s2 = {
		14, 29,
		0.25, 0.5
	};
	
	print_overlap_info(s1, s2);
	
	return 0;
}
