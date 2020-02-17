#ifndef THREE_D_TYPES_H
#define THREE_D_TYPES_H 1

#include <iostream>

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

std::ostream& operator<< (std::ostream &o, vert_shaded const& v);


struct tri {
	int ind[3]; //Indices into vertex array
	int frame; //Keeps track of last frame when triangle gradients were updated
	float gnx_z_x, gnx_z_y; //Gradient of nx/z
	float gny_z_x, gny_z_y; //Gradient of ny/z
	float gnz_z_x, gnz_z_y; //Gradient of nz/z
	
	float gz_inv_x, gz_inv_y; //Gradient of z^-1
};

#endif
