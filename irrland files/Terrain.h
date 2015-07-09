#pragma once

#define NBHARMONIC 32

#define BB 0x100
#define BM 0xff

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ( t * t * (3.f - 2.f * t) )
#define lerp(t, a, b) ( a + t * (b - a) )
#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.f;

// real terrain emulation
#define MAX_LAT 47
#define MIN_LAT 43
#define MAX_LON 8
#define MIN_LON 4
#define DATABLOCK_NBS 1200

#ifndef Point 
struct Point {
	int x,y;
};
#endif

struct coordWGS84 {
	float lat,lon;
};
//

class CTerrain
{
public:
	CTerrain(void);
	~CTerrain(void);
	float CTerrain::getAltitude(float vec[]);
	void CTerrain::reset(void);
	int CTerrain::myRand();
	void CTerrain::mySRand(unsigned int seed);

	Point getXYFromWGS84(float lon, float lat);
	coordWGS84 getCoordsFromXY(float x, float z);
	float CTerrain::getAltitude(float x, float y);

protected:
	unsigned int m_randSeed;
	int start;
	int p[BB + BB + 2];
	float g3[BB + BB + 2][3];
	float g2[BB + BB + 2][2];
	float g1[BB + BB + 2];

	double CTerrain::noise1(double arg);
	float CTerrain::noise2(float vec[2]);
	float CTerrain::noise3(float vec[3]);
	void CTerrain::normalize2(float v[2]);
	void CTerrain::normalize3(float v[3]);
	void CTerrain::init(void);
};
