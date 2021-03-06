#include "preproc.hpp"

#include <blocks.hpp>
#include <generator.h>
#include <genthread.h>
#include <biome/biome.hpp>
#include <random.hpp>
#include <sectors.hpp>
#include "objects/volumetrics.hpp"

void preProcess(genthread_t* l_thread)
{
	const int miny = 0;
	const int maxy = GEN_FULLHEIGHT;
	
	// world coordinates
	int wx = l_thread->x;
	int wz = l_thread->z;
	
	// absolute block coords
	int x = wx * Sector::BLOCKS_XZ;
	int z = wz * Sector::BLOCKS_XZ;
	
	Flatland& flat = flatlands(wx, wz);
	int terrain;
	
	int dx, dy, dz;
	block *lastb, *b;
	block airblock; airblock.id = 0;
	int counter;
	int air, treecount;
	
	for (dx = x; dx < x + Sector::BLOCKS_XZ; dx++)
	{
		for (dz = z; dz < z + Sector::BLOCKS_XZ; dz++)
		{
			// get terrain id
			terrain = flat(dx & (Sector::BLOCKS_XZ-1), 
					dz & (Sector::BLOCKS_XZ-1)).terrain;
			
			lastb = getb(dx, maxy+1, dz); // get top block, just in case (99.99% _AIR)
			if (lastb == 0) lastb = &airblock; // prevent null pointer in this case
			
			// start counting from top (pretend really high)
			counter = GEN_FULLHEIGHT; // - (maxy + 1);
			air = counter; // used to determine light!
			treecount = 0;
			
			for (dy = maxy; dy >= miny; dy--)
			{
				b = getb(dx, dy, dz);
				
				if (isLeaf(b->id)) treecount++;
				
				// check if ultradifferent
				if (b->id != lastb->id)
				{
					// check if megatransparent
					if (blockTransparent(lastb->id))
					{
						// if we get here, the block above was transparent (air etc.)
						// air determines how many transparent blocks were traversed
						// counter determines how many blocks of THE SAME ID were traversed
						
						if (dy > GEN_WATERBLOCKS)
						{
							// we hit soil, and at least some air
							if (b->id == _GREENSOIL && air > 3)
							{
								float rand = randf(dx, dy, dz);
								// fill potential volume (above)
								if (rand > 0.9)
								{
									volumetricFill(dx, dy, dz, _WATER);
								}
								
							} // air to soil
						} // > water level
						
					}
					
					// remember this id! and reset counters!
					lastb = b;
					counter = 0;
					
				}  // id != lastid
				else
				{  // id == lastid
					counter++; // same id, count!!
				}
				
				// count air, and only reset after we have possibly hit something
				if (blockTransparent(b->id)) air++; else air = 0;
				
			} // y
			
		} // next z
		
	} // next x
	
} // preProcess()
