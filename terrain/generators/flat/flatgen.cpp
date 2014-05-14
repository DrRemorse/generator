/**
 * Flat world generator
 * 
**/
#include "generator.h"
#include "genthread.h"
#include <sectors.hpp>
#include <blocks.hpp>
#include "biome/biome.hpp"
#include "noise/simplex1234.h"
//#include "random.hpp"

// flat terrain generation function

void flatTerrain(genthread* l_thread)
{
	Sector* s = nullptr;
	int wx = l_thread->x, wz = l_thread->z;
	int x, y, z;
	vec3 p;
	
	int by; // local sector coordinate (0 to 7)
	unsigned short id = _AIR;
	
	// some noise
	const f64_t grid_wfac = 1.0 / (f64_t)(Sector::BLOCKS_XZ * Sector::BLOCKS_XZ);
	float simplex[Sector::BLOCKS_XZ][Sector::BLOCKS_XZ];
	
	for (x = 0; x < Sector::BLOCKS_XZ; x++)
	for (z = 0; z < Sector::BLOCKS_XZ; z++)
	{
		p.x = l_thread->p.x + (f64_t)x * grid_wfac;
		p.z = l_thread->p.z + (f64_t)z * grid_wfac;
		simplex[x][z] = 0.5 + snoise2(p.x, p.z) * 0.25;
	}
	
	for (y = GEN_FULLHEIGHT; y >= 0; y--)
	{
		// world vec.y
		p.y = (f64_t)y / (f64_t)GEN_FULLHEIGHT;
		
		// internal sector coordinate
		by = y & (Sector::BLOCKS_Y-1);
		// if at the top of a new sector, get sector pointer
		if (by == Sector::BLOCKS_Y-1) s = &sectors(wx, y >> 3, wz);
		
		for (x = 0; x < Sector::BLOCKS_XZ; x++)
		{
			for (z = 0; z < Sector::BLOCKS_XZ; z++)
			{
				/*
					Make a flat world
				*/
				id = _AIR;
				float h = simplex[x][z];
				
				if (p.y < h - 0.15) id = _STONE;
				else
				if (p.y < h) id = _GREENSOIL;
				
				// if the block is not _AIR (0), set the block
				if (id) setsimple(s, x, by, z, id);
				
			} // z
			
		} // x
		
	} // y
	
}
