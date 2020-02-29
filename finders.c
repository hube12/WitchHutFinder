#include "finders.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>


//==============================================================================
// Globals
//==============================================================================


Biome biomes[256];

Pos getStructurePos(const StructureConfig config, int64_t seed, const int regionX, const int regionZ) {
	Pos pos;

	// set seed
	seed = regionX * 341873128712 + regionZ * 132897987541 + seed + config.seed;
	seed = ((uint64_t )seed ^ 0x5deece66dLLu);// & ((1LL << 48) - 1);

	seed = (seed * 0x5deece66dLLu + 0xbLLu) & 0xffffffffffffu;

	if ((uint64_t )config.properties & USE_POW2_RNG) {
		// Java RNG treats powers of 2 as a special case.
		pos.x = (int)(((uint64_t)config.chunkRange * ((uint64_t)seed >> 17u)) >> 31u);

		seed = (seed * 0x5deece66dLL + 0xbLLu) & 0xffffffffffffu;
		pos.z = (int)(((uint64_t)config.chunkRange * ((uint64_t)seed >> 17u)) >> 31u);
	} else {
		pos.x = (int) ((uint64_t )seed >> 17u) % config.chunkRange;

		seed = (seed * 0x5deece66dLLu + 0xbLLu) & 0xffffffffffffu;
		pos.z = (int) ((uint64_t)seed >> 17u) % config.chunkRange;
	}

	pos.x = (int)((uint64_t )(regionX * config.regionSize + pos.x) << 4u) + 8;
	pos.z = (int)((uint64_t )(regionZ * config.regionSize + pos.z) << 4u) + 8;
	return pos;
}

int getBiomeAtPos(const LayerStack g, const Pos pos) {
	int *map = allocCache(&g.layers[g.layerNum - 1], 1, 1);
	genArea(&g.layers[g.layerNum - 1], map, pos.x, pos.z, 1, 1);
	int biomeID = map[0];
	free(map);
	return biomeID;
}


