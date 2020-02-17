#ifndef POLY_RASTER_H
#define POLY_RASTER_H 1

void scan_edge(int x0, int y0, int x1, int y1, int lr, int skip_first, std::vector<int> &out);
void scan_tri(std::vector<int> &lefts, std::vector<int> &rights, int& top_y, vert_shaded const *V, tri const& t, int verbose = 0);

#endif
