#ifndef MAT4_H
#define MAT4_H

#include "kbox.h"
#include "sinlut.h"

/*---- types ----*/
typedef struct mat4
{
  f32 m[4][4];
} mat4;

/*---- functions ----*/
INLINE mat4 mat4_identity()
{
	mat4 a = { .m={ 
		{s32tof32(1),0,0,0},
		{0,s32tof32(1),0,0},
		{0,0,s32tof32(1),0},
		{0,0,0,s32tof32(1)}
	}};
	return a;
}

INLINE mat4 mat4_mulM(mat4 *a,mat4 *b)
{
  mat4 c = { .m = {} };
  for(int y=0; y<4; y++)
	{
		for(int x=0; x<4; x++)
		{
			for(int p=0; p<4; p++)
			{ c.m[y][x] += mulf32(a->m[y][p],b->m[p][x]); }
		}
	}
  return c;
}
INLINE vec3_f32 mat4_mulV(mat4 *a,vec3_f32 *b)
{
	vec3_f32 c = {
		.x = mulf32(a->m[0][0],b->x) + mulf32(a->m[0][1],b->y) + mulf32(a->m[0][2],b->z) + a->m[0][3],
		.y = mulf32(a->m[1][0],b->x) + mulf32(a->m[1][1],b->y) + mulf32(a->m[1][2],b->z) + a->m[0][3],
		.z = mulf32(a->m[2][0],b->x) + mulf32(a->m[2][1],b->y) + mulf32(a->m[2][2],b->z) + a->m[0][3],
	};

	return c;
}

INLINE mat4 mat4_ang(u32 rx,u32 ry,u32 rz)
{
	mat4 xm = {.m={
		{s32tof32(1),0,0},
		{0,lu_cos(rx),-lu_sin(rx)},
		{0,lu_sin(rx),lu_cos(rx)},
		{0,0,0,s32tof32(1)}
	}};

	mat4 ym = {.m={
		{lu_cos(ry),0,lu_sin(ry)},
		{0,s32tof32(1),0},
		{-lu_sin(ry),0,lu_cos(ry)},
		{0,0,0,s32tof32(1)}
	}};

	mat4 zm = {.m={
		{lu_cos(rz),-lu_sin(rz),0},
		{lu_sin(rz),lu_cos(rz),0},
		{0,0,s32tof32(1)},
		{0,0,0,s32tof32(1)}
	}};

	mat4 m1 = mat4_mulM(&xm,&ym);
	mat4 m2 = mat4_mulM(&m1,&zm);
	return m2;
}

/*
mat3.ang = function(x,y,z)
	local c,s = cos,sin
	local xm = mat3.new(
		{1,0,0},
		{0,c(x),-s(x)},
		{0,s(x),c(x)}
	)
	local ym = mat3.new(
		{c(y),0,s(y)},
		{0,1,0},
		{-s(y),0,c(y)}
	)
	local zm = mat3.new(
		{c(z),-s(z),0},
		{s(z),c(z),0},
		{0,0,1}
	)
	return xm*ym*zm
end
*/

#endif
