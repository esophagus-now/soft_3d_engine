#include "3d_types.h"
#include "main.h"
#include <vector>

using namespace std;

ostream& operator<< (ostream &o, vert_shaded const& v) {
	return o << "<" << v.x << "," << v.y << ">" << el;
}
