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
#include "3d_math.h"
#include "3d_types.h"
#include "poly_raster.h"
#include "sbuf.h"

using namespace std; 

std::ostream& el(std::ostream &o) {
	return o << "\n";
}

vert cube_verts[] = {
	//Bottom
	{{-1.0,  1.0, -3.0}}, //Back-left
	{{ 1.0,  1.0, -3.0}}, //Back-right
	{{-1.0, -1.0, -3.0}}, //Front-left
	{{ 1.0, -1.0, -3.0}}, //Front-right
				
	//Top       
	{{-1.0,  1.0, -1.0}}, //Back-left
	{{ 1.0,  1.0, -1.0}}, //Back-right
	{{-1.0, -1.0, -1.0}}, //Front-left
	{{ 1.0, -1.0, -1.0}}  //Front-right
};

vert_shaded cube_shaded[sizeof(cube_verts)/sizeof(*cube_verts)];

#define BBL 0
#define BBR 1
#define BFL 2
#define BFR 3
#define TBL 4
#define TBR 5
#define TFL 6
#define TFR 7

//TODO: Change tri to:
//	int pos_ind[3]
//	int norm_ind[3]
tri cube[] = {
	//Bottom
	{{BBL,BFR,BFL}, 255, 0, 0},
	{{BBL,BBR,BFR}, 255, 0, 0},
	//Back
	{{BBR,TBL,TBR}, 0, 255, 0},
	{{BBR,BBL,TBL}, 0, 255, 0},
	//Left
	{{BFL,TBL,BBL}, 0, 0, 255},
	{{BFL,TFL,TBL}, 0, 0, 255},
	//Front
	{{BFL,TFR,TFL}, 255,255,255},
	{{BFL,BFR,TFR}, 255,255,255},
	//Right
	{{BFR,TBR,TFR}, 0, 200, 200},
	{{BFR,BBR,TBR}, 0, 200, 200},
	//Top
	{{TFL,TBR,TBL}, 200, 0, 200},
	{{TFL,TFR,TBR}, 200, 0, 200},
};

int main(int argc, char **argv) {
	vert_shaded testverts[4];
	testverts[0].x = 0; testverts[0].y = 40;
	testverts[1].x = 20; testverts[1].y = 0;
	testverts[2].x = 30; testverts[2].y = 30;
	testverts[3].x = 10; testverts[3].y = 80;
	
	tri t1; t1.ind[0] = 0; t1.ind[1] = 1; t1.ind[2] = 2;
			t1.r = 0; t1.g = 0; t1.b = 0;
	tri t2; t2.ind[0] = 0; t2.ind[1] = 2; t2.ind[2] = 3;
			t2.r = 255; t2.g = 0; t2.b = 0;
	
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
	int edit_index = 0;
	
	xrot xr(0.0);
	yrot yr(0.0);
	scale scl(0.1, 0.1, 0.1);
	
	vector<xformable const*> mv;
	mv.reserve(3);
	mv.push_back(&xr);
	mv.push_back(&yr);
	mv.push_back(&scl);
	
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
					line[j].r = 0;
					line[j].g = 0;
					line[j].b = 0;
				}
			}
			
			shade_vert_arr(cube_shaded, cube_verts, get_xform(mv), sizeof(cube_verts)/sizeof(*cube_verts));
			
			//Draw triangles!!
			sbuffer<HEIGHT> s;
			for (auto const& t: cube) {
				s.insert_tri(t, cube_shaded);
			}
			
			s.draw(surf);
			SDL_UpdateWindowSurface(win);
			
			unsigned numFrames = accum_error/33;
			accum_error %= 33;
			fnum = (fnum + numFrames) % 33;
			
			float ang = xr.get_deg();
			ang += (float) numFrames;
			if (ang > 360.0) ang -= 360.0;
			xr.set_deg(ang);
			
			ang = yr.get_deg();
			ang +=  1.5 *(float) numFrames;
			if (ang > 360.0) ang -= 360.0;
			yr.set_deg(ang);
			
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
	
	xform x;
	
	return 0;
}
