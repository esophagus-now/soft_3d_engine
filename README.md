# soft_3d_engine
Software 3D engine written in C++ and using SDL. Demonstrates the S-buffer technique

I made this ages ago, but ran out of free time to add shading. The technique was based off some paper I found which explained perspective-correct interpolation, but I've lost the link and I'll need to find it again. Basically, any quantity you want to interpolate over a triangle will not be linear in the x and y screen coordinates, but it does turn out to be linear in z? I can't really remember the details, but the technique was to linearly interpolate z inverse in the screen coordinates then invert it to get interpolated z, then your desired quantity was some linear transformation of the z value?

Also, you used to be able to serach "S-buffer" on google and fidn this forum post on some game dev website where someone explained the technique. I can't seem to find it anymore, so I'll have to poke through wayback machine or something.

Too bad I stopped this project dead. I wanted to work on it today, and I did leave some pretty good comments, but there's too much I can't quite remember.
