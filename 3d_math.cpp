#include <vector>
#include <cmath>
#include "main.h"
#include "3d_types.h"
#include "3d_math.h"

using namespace std;

xform get_xform(std::vector<xformable const *> const& v) {
	xform cur = ident();
	
	for(auto it = v.crbegin(); it != v.crend(); ++it) {
		cur *= *(*it);
	}
	
	return cur;
}

//TODO: Should we start using integers for z_inv? It should be possible...
//...but I have no reason to think it's necessary or even better.
void shade_vert(vert_shaded &dst, vert const& src, xform const& mv, float zfov, float screen_scale) {
	pnt pos = mv * src.pos;
	pnt norm = mv * src.norm;
	
	float persp_calc = zfov/(zfov-pos[2]);
	
	dst.x = (short)(screen_scale*(persp_calc*pos[0] + 0.5));
	dst.y = (short)(screen_scale*(persp_calc*pos[1] + 0.5));
	dst.z_inv = 1.0/(zfov - pos[2]); //Depth of this point
	dst.norm_z = norm*dst.z_inv;
}

void shade_vert_arr(vert_shaded *dest, vert const *src, xform const& mv, int len) {
	for (int i = 0; i < len; i++) {
		shade_vert(dest[i], src[i], mv, FOV, HEIGHT);
	}
}
