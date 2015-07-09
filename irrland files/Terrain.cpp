#include "terrain.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


CTerrain::CTerrain(void)
{
	mySRand((unsigned int)time(NULL));
	init();
}

CTerrain::~CTerrain(void)
{
}

float CTerrain::getAltitude(float x, float y)
{
	float vec[2]={x,y};
	return getAltitude(vec);
}

float CTerrain::getAltitude(float vec[])
{
	float harmovec[2];
	float f=0;
	for(float h=4.0f; h<=NBHARMONIC; h*=2.0f)
	{
		harmovec[0] = vec[0]*h;
		harmovec[1] = vec[1]*h;
		f+=noise2(harmovec)/h; //*2.9f /*bordel factor*/;
	}
	f=(f+0.1f /*more or less sea*/ )*6000.0f; /* top height */

	return f;
}

/* coherent noise function over 1, 2 or 3 dimensions */
/* (copyright Ken Perlin) */
double CTerrain::noise1(double arg)
{
	int bx0, bx1;
	float rx0, rx1, sx, t, u, v, vec[1];
	vec[0] = (float)arg;	// optimisation : le cast est pas gratuit! à virer 

	setup(0, bx0,bx1, rx0,rx1);

	sx = s_curve(rx0);

	u = rx0 * g1[ p[ bx0 ] ];
	v = rx1 * g1[ p[ bx1 ] ];

	return lerp(sx, u, v);
}

void CTerrain::reset(void)
{
	start=1;
	init();
}

float CTerrain::noise2(float vec[2])
{
	int bx0, bx1, by0, by1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	sx = s_curve(rx0);
	sy = s_curve(ry0);

#define at2(rx,ry) ( rx * q[0] + ry * q[1] )

	q = g2[ b00 ] ; u = at2(rx0,ry0);
	q = g2[ b10 ] ; v = at2(rx1,ry0);
	a = lerp(sx, u, v);

	q = g2[ b01 ] ; u = at2(rx0,ry1);
	q = g2[ b11 ] ; v = at2(rx1,ry1);
	b = lerp(sx, u, v);

	return lerp(sy, a, b);
}

float CTerrain::noise3(float vec[3])
{
	int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
	float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
	register int i, j;

	setup(0, bx0,bx1, rx0,rx1);
	setup(1, by0,by1, ry0,ry1);
	setup(2, bz0,bz1, rz0,rz1);

	i = p[ bx0 ];
	j = p[ bx1 ];

	b00 = p[ i + by0 ];
	b10 = p[ j + by0 ];
	b01 = p[ i + by1 ];
	b11 = p[ j + by1 ];

	t  = s_curve(rx0);
	sy = s_curve(ry0);
	sz = s_curve(rz0);

	#define at3(rx,ry,rz) ( rx * q[0] + ry * q[1] + rz * q[2] )

	q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
	q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
	a = lerp(t, u, v);

	q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
	q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
	b = lerp(t, u, v);

	c = lerp(sy, a, b);

	q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
	q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
	a = lerp(t, u, v);

	q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
	q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
	b = lerp(t, u, v);

	d = lerp(sy, a, b);

	return lerp(sz, c, d);
}

void CTerrain::normalize2(float v[2])
{
	float s;

	s = (float)sqrt(v[0] * v[0] + v[1] * v[1]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
}

void CTerrain::normalize3(float v[3])
{
	float s;

	s = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] = v[0] / s;
	v[1] = v[1] / s;
	v[2] = v[2] / s;
}

void CTerrain::mySRand(unsigned int seed)
{
	m_randSeed=seed;
}

inline int CTerrain::myRand()
{
	// -- dom random
	//return (m_randSeed = (1531664525L * m_randSeed + 1013904223L) % RAND_MAX);

	// Mr bee's random
	return (m_randSeed = 1664525L * m_randSeed + 1013904223L) >> 5;

	/* -- Pierre Larbier's random --
	long haut , bas , inter , q;
	bas = 16807 * (m_randSeed & 0xffffL);
	haut = 16807 * (m_randSeed >> 16);
	inter = (bas >> 16) + haut;
	bas = ((bas & 0xffff) | ((inter & 0x7fff)<<16)) + (inter >> 15);
	if ((bas & 0x80000000L) != 0)
		bas = (bas + 1) & 0x7fffffffL;
	m_randSeed = bas;
	return bas;
	*/

	//-- msvc++ random
	//return rand();
}


void  CTerrain::init(void)
{
	int i, j, k;

	for (i = 0 ; i < BB ; i++) {
		p[i] = i;
		g1[i] = (float)((myRand() % (BB + BB)) - BB) / BB;
		for (j = 0 ; j < 2 ; j++)
			g2[i][j] = (float)((myRand() % (BB + BB)) - BB) / BB;

		normalize2(g2[i]);

		for (j = 0 ; j < 3 ; j++)
			g3[i][j] = (float)((myRand() % (BB + BB)) - BB) / BB;
		normalize3(g3[i]);
	}

	while (--i) {
		k = p[i];
		p[i] = p[j = myRand() % BB];
		p[j] = k;
	}

	for (i = 0 ; i < BB + 2 ; i++) {
		p[BB + i] = p[i];
		g1[BB + i] = g1[i];
		for (j = 0 ; j < 2 ; j++)
			g2[BB + i][j] = g2[i][j];
		for (j = 0 ; j < 3 ; j++)
			g3[BB + i][j] = g3[i][j];
	}
}

//---------------------------------------------------------------------------------------------

// lon/lat -> x,y
Point CTerrain::getXYFromWGS84(float lon, float lat)
{
	Point p;
	p.x=0;
	p.y=0;
	if( lon < MIN_LON || lon> MAX_LON )
		return p;

	if( lat < MIN_LAT || lat > MAX_LAT )
		return p;

	// --- coords dans le segment de data
	p.x=(int)((lon - (float)MIN_LON)*(float)DATABLOCK_NBS);
	p.y=(int)(((float)MAX_LAT - lat)*(float)DATABLOCK_NBS);

	return p;
}

// x,y -> lon/lat
// beware, it's ABSOLUTE x,y in datablock, not screen coordinates (-> add offset)
coordWGS84 CTerrain::getCoordsFromXY(float x, float z)
{
	coordWGS84 coord;
	coord.lon=(float)MIN_LON + x/DATABLOCK_NBS;
	coord.lat=(float)MAX_LAT - z/DATABLOCK_NBS;
	return coord;
}
