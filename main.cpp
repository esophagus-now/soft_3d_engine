#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <cstddef>
#include <utility> //swap
#include <SDL.h>
#include <unistd.h>

#define DEBUG 1

#if (DEBUG == 1)
#define DBG_PRINT(x) cout << #x " = " << x << el
#define DBG_PUTS(x) cout << x << el
#else
#define DBG_PRINT(x)
#define DBG_PUTS(x)
#endif

using namespace std; 

ostream& el(ostream &o) {
	return o << "\n";
}

template <typename T>
ostream& operator<<(ostream &o, vector<T> const& v) {
	o << "[";
	auto delim = "";
	for (auto const& i : v) {
		o << delim << i;
		delim = ", ";
	}
	return o << "]";
}

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

typedef unsigned char byte;

#pragma pack(1)
struct bmp_pixel {
	byte b, g, r;
};

#pragma pack(1)
struct sdl_pixel {
	byte b, g, r;
	byte unused;
};

#pragma pack(1)
struct bmp {
	//Header
	char     sig[2];			//Signature
	unsigned f_sz;				//File size in bytes
	unsigned unused;			//Unused
	unsigned data_off;			//Offset of data from start of file
	
	//Info header
	unsigned info_sz;			//Size of info header
	unsigned width;				//Width of image (in pixels)
	unsigned height;				//Height of image (in pixels)
	unsigned short planes;		//Number of planes? Just set this to 1?
	unsigned short bpp;			//Bits per pixel
	unsigned cmp;				//Type of compression. 0 for none
	unsigned img_sz;			//Valid to set to 0 if no compression
	unsigned x_px_per_m;		//Horizontal resolution (px/meter)
	unsigned y_px_per_m;		//Vertical resolution (px/meter)
	unsigned cols_used;			//Number of colours actually used
	unsigned imp_cols;			//Number of important colours (0 = all)
	
	//After this is an optional colormap if you use less than 16 bpp. I 
	//don't use it
};

#define WIDTH 400
#define HEIGHT 400

#define SCANLINE_SZ (4*((WIDTH*sizeof(bmp_pixel)+3)/4))

//Model-space coordinates and other info of a vertex
//These would come from Blender
struct vert {
	float x, y, z;
	float nx, ny, nz;
	float r, g, b; //?
	int u, v; //Texture coordinates
};

//[v1, v2, v3, v4, ..., vn] //Original vertices
//[V1, V2, V3, V4, ..., Vn] //Shaded vertices
// tri = [i, j, k], means triangle is defined by vi, vj, and vk

struct vert_shaded {
	int x, y;
	float z_inv; // (1/z)
	float nx_z, ny_z, nz_z; //Transformed normal, divded by z
	int u, v; //Texture coordinates. Only ever copied once; then they are just left there
};

struct tri {
	int ind[3]; //Indices into vertex array
	int frame; //Keeps track of last frame when triangle gradients were updated
	float gnx_z_x, gnx_z_y; //Gradient of nx/z
	float gny_z_x, gny_z_y; //Gradient of ny/z
	float gnz_z_x, gnz_z_y; //Gradient of nz/z
	
	float gz_inv_x, gz_inv_y; //Gradient of z^-1
};

struct segment {
	int x1, x2;
	tri const *t; //Gives us access to gradients
};

template <int N_SCANLINES>
struct sbuffer {
	vector<segment> scanlines[N_SCANLINES];
	
	
};

void scanEdge(int x0, int y0, int x1, int y1, int lr, int skip_first, vector<int> &out) {
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

void scanTri(vector<int> &lefts, vector<int> &rights, int& top_y, vert_shaded const *V, tri const& t) {
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
			DBG_PUTS("Case 1");
			top_y = v3.y;
			scanEdge(v3.x, v3.y, v2.x, v2.y, 1, 0, lefts);
			scanEdge(v2.x, v2.y, v1.x, v1.y, 1, 0, lefts);
			scanEdge(v3.x, v3.y, v1.x, v1.y, 0, 0, rights);
		} else if (c < a) {
			//b < a, b<= c, c < a
			//b <= c < a
			//B top, A bottom, check pointiness
			DBG_PUTS("Case 2");
			top_y = v2.y;
			int top_is_pointy = (v2.y == v3.y);
			scanEdge(v2.x, v2.y, v1.x, v1.y, 1, top_is_pointy, lefts);
			scanEdge(v2.x, v2.y, v3.x, v3.y, 0, top_is_pointy, rights);
			scanEdge(v3.x, v3.y, v1.x, v1.y, 0, 0, rights);
		} else {
			//b < a, b <= c, a <= c
			//b < a <= c
			//B top, A bottom
			DBG_PUTS("Case 3");
			top_y = v2.y;
			scanEdge(v2.x, v2.y, v1.x, v1.y, 1, 0, lefts);
			scanEdge(v1.x, v1.y, v3.x, v3.y, 1, 0, lefts);
			scanEdge(v2.x, v2.y, v3.x, v3.y, 0, 0, rights);
		}
	} else {
		if (c < a) {
			//a <= b, c < a
			//c < a <= b
			//C top, B bottom
			DBG_PUTS("Case 4");
			top_y = v3.y;
			scanEdge(v3.x, v3.y, v2.x, v2.y, 1, 0, lefts);
			scanEdge(v3.x, v3.y, v1.x, v1.y, 0, 0, rights);
			scanEdge(v3.x, v3.y, v2.x, v2.y, 0, 0, rights);
		} else if (c < b) {
			//a <= b, a <= c, c < b
			//a <= c < b
			//A top, B bottom, check pointiness
			DBG_PUTS("Case 5");
			top_y = v1.y;
			int top_is_pointy = (v3.y == v1.y);
			scanEdge(v1.x, v1.y, v3.x, v3.y, 1, top_is_pointy, lefts);
			scanEdge(v3.x, v3.y, v2.x, v2.y, 1, 0, lefts);
			scanEdge(v1.x, v1.y, v2.x, v2.y, 0, top_is_pointy, rights);
		} else {
			//a <= b, a <= c, b <= c
			//a <= b <= c
			//A top, C bottom, check pointiness
			DBG_PUTS("Case 6");
			top_y = v1.y;
			int top_is_pointy = /*(v2.y == v1.y)*/ 1;
			scanEdge(v1.x, v1.y, v3.x, v3.y, 1, top_is_pointy, lefts);
			scanEdge(v1.x, v1.y, v2.x, v2.y, 0, top_is_pointy, rights);
			scanEdge(v2.x, v2.y, v3.x, v3.y, 0, 0, rights);
		}
	}
	
	//Calculate gradients
	
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
	while (1) {
		unsigned thisTime = SDL_GetTicks();
		unsigned delta = thisTime - lastTime;
		lastTime = thisTime;
		
		accum_error += delta;
		
		if (accum_error > 33) {
			DBG_PUTS("\nNEW FRAME");
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
			#if (DEBUG == 1)
			if (draw_first == &t1) DBG_PUTS("Drawing black triangle");
			else DBG_PUTS("Drawing red triangle");
			#endif
			scanTri(lefts, rights, top_y, testverts, *draw_first);
			
			for (int i = 0, y = top_y; i < int(lefts.size()); i++, y++) {
				sdl_pixel *line = (sdl_pixel *) ((char*)(surf->pixels) + y*surf->pitch);
				for (int x = lefts[i]; x <= rights[i]; x++) {
					line[x].r = (draw_first == &t2)*255;
					line[x].g = 0;
					line[x].b = 0;
				}
			}
			
			lefts.clear(); rights.clear();
			#if (DEBUG == 1)
			if (draw_second == &t1) DBG_PUTS("Drawing black triangle");
			else DBG_PUTS("Drawing red triangle");
			#endif
			scanTri(lefts, rights, top_y, testverts, *draw_second);
			
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
					if (testverts[0].y > 0) testverts[0].y--;
					break;
				case SDLK_DOWN:
					if (testverts[0].y < HEIGHT-1) testverts[0].y++;
					break;
				case SDLK_LEFT:
					if (testverts[0].x > 0) testverts[0].x--;
					break;
				case SDLK_RIGHT:
					if (testverts[0].x < WIDTH-1) testverts[0].x++;
					break;
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
	
	return 0;
}
