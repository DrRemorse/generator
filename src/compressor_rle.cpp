#include "compressor_rle.hpp"

#include <library/log.hpp>
#include <cstring>

using namespace library;
RLECompressor rle;

RLECompressor::RLECompressor()
{
	rh.palettes = 0;
	rh.entries = 0;
	this->decomp = nullptr;
	this->rledata = nullptr;
}

void RLECompressor::zero()
{
	// sectorblock info
	rh.commonData = 0;
	// rle data
	rh.palettes = 0;
	rh.entries  = 0;
}

/// DECOMPRESSOR ///

void RLECompressor::compress(Sector::sectorblock_t& sb)
{
	// set common data
	rh.commonData = sb.blocks + (sb.torchlight << 12) + (sb.hardsolid << 24);
	
	// create palette (512 bytes)
	block_t blocks[256];
	// number of unique blocks in sectorblock
	unsigned short pals = 0;
	// the palette entries as we iterate sector
	unsigned short entries = 0;
	unsigned short lastentry = 65535;
	
	// first and last block in sector
	block_t* first = sb.b;
	block_t* last  = first + Sector::BLOCKS_XZ * Sector::BLOCKS_XZ * Sector::BLOCKS_Y;
	// iterate sector & create palette, entries
	for (block_t* b = first; b < last; b++)
	{
		bool found = false;
		
		for (unsigned short p = 0; p < pals; p++)
		{
			if (b[0] == blocks[p])
			{
				// an existing palette entry was found
				found = true;
				// set the palette entry now
				b[0] = p;
				break;
			}
		}
		
		if (found == false)
		{
			// add block to palette
			blocks[pals] = b[0];
			// cheat, set block id to palette entry
			b[0] = pals;
			// increase palette count
			pals ++;
		}
		
		// now we will re-use the block id to count number of entries
		if (b[0] != lastentry)
		{
			lastentry = b[0];
			// the palette id changed, so we need another entry
			entries += 1;
		}
	}
	
	// set rle data
	rh.palettes = pals;
	rh.entries  = entries;
	
	// create data entries and palette in same operation
	delete[] this->rledata;
	this->rledata = new char[entries * sizeof(RLEEntry) + pals * sizeof(block_t)];
	
	// copy over palette
	memcpy(this->rledata + entries * sizeof(RLEEntry), blocks, pals * sizeof(block_t));
	
	// loop through blocks again creating data entries
	lastentry = 65535;
	RLEEntry* centry = ((RLEEntry*) this->rledata) - 1;
	
	for (block_t* palid = first; palid < last; palid++)
	{
		if (palid[0] == lastentry)
		{
			// same entry as before
			// increase entry count for last entry
			centry->count ++;
		}
		else
		{
			lastentry = palid[0];
			
			// create new entry
			centry ++;
			centry->palid = palid[0];
			centry->count = 1;
		}
	}
	
} // RLE compress()

/// COMPRESSOR ///

bool RLECompressor::hasBlocks(unsigned char* data)
{
	// set RLEHeader internally, so that further info can be retrieved later on
	// CAREFUL when removing the next line, there is a dependency in the compressor
	decomp = (RLEHeader*) data;
	
	return decomp->getBlocks() != 0;
}

void RLECompressor::decompress(unsigned char* data, Sector::sectorblock_t& sb)
{
	// first treat data as a RLEHeader
	decomp = (RLEHeader*) data;
	data += sizeof(RLEHeader);
	
	// get entry data
	RLEEntry* entry = (RLEEntry*) data;
	data += decomp->getEntries() * sizeof(RLEEntry);
	
	// first palette (block)
	block_t* palette = (block_t*) data;
	// last entry in palette entries list (used as count)
	RLEEntry* entries = entry + decomp->getEntries();
	// entry point to sector blocks
	block_t* sblock = sb.b;
	
	for (; entry < entries; entry++)
	{
		block_t blockData = palette[ entry->palid ];
		while (entry->count--)
		{
			*sblock = blockData; sblock++;
		}
	}
	// set sectorblock info
	sb.blocks     = decomp->getBlocks();
	sb.torchlight = decomp->getLights();
	sb.hardsolid  = decomp->getHardsolid();
}
