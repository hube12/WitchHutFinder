#ifndef FINDERS_H_
#define FINDERS_H_

#include "generator.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


#define SEED_BASE_MAX (1LL << 48)
#define PI 3.141592653589793

#define LARGE_STRUCT 1u
#define USE_POW2_RNG 2u

enum {
    Desert_Pyramid, Igloo, Jungle_Pyramid, Swamp_Hut,
    Village, Ocean_Ruin, Shipwreck, Monument, Mansion
};

STRUCT(StructureConfig) {
    int64_t seed;
    int regionSize, chunkRange;
    int properties;
};

/* For desert temples, igloos, jungle temples and witch huts prior to 1.13. */
static const StructureConfig FEATURE_CONFIG        = { 14357617, 32, 24, 0};

/* 1.13 separated feature seeds by type */
static const StructureConfig DESERT_PYRAMID_CONFIG = { 14357617, 32, 24, 0};
static const StructureConfig IGLOO_CONFIG          = { 14357618, 32, 24, 0};
static const StructureConfig JUNGLE_PYRAMID_CONFIG = { 14357619, 32, 24, 0};
static const StructureConfig SWAMP_HUT_CONFIG      = { 14357620, 32, 24, 0};

static const StructureConfig VILLAGE_CONFIG        = { 10387312, 32, 24, 0};
static const StructureConfig OCEAN_RUIN_CONFIG     = { 14357621, 16,  8, USE_POW2_RNG};
static const StructureConfig SHIPWRECK_CONFIG      = {165745295, 15,  7, 0};
static const StructureConfig MONUMENT_CONFIG       = { 10387313, 32, 27, LARGE_STRUCT};
static const StructureConfig MANSION_CONFIG        = { 10387319, 80, 60, LARGE_STRUCT};

static const int templeBiomeList[] = {desert, desertHills, jungle, jungleHills, swampland, icePlains, coldTaiga};
static const int biomesToSpawnIn[] = {forest, plains, taiga, taigaHills, forestHills, jungle, jungleHills};
static const int oceanMonumentBiomeList1[] = {ocean, deepOcean, river, frozenRiver, frozenOcean, frozenDeepOcean, coldOcean, coldDeepOcean, lukewarmOcean, lukewarmDeepOcean, warmOcean, warmDeepOcean};
static const int oceanMonumentBiomeList2[] = {frozenDeepOcean, coldDeepOcean, deepOcean, lukewarmDeepOcean, warmDeepOcean};
static const int villageBiomeList[] = {plains, desert, savanna, taiga};
static const int mansionBiomeList[] = {roofedForest, roofedForest+128};

STRUCT(Pos)
{
    int x, z;
};


//==============================================================================
// Globals
//==============================================================================

extern Biome biomes[256];



//==============================================================================
// Finding Structure Positions
//==============================================================================

/* Fast implementation for finding the block position at which the structure
 * generation attempt will occur within the specified region.
 * This function applies for scattered-feature structureSeeds and villages.
 */
Pos getStructurePos(const StructureConfig config, int64_t seed,
        const int regionX, const int regionZ);

//==============================================================================
// Checking Biomes & Biome Helper Functions
//==============================================================================

/* Returns the biome for the specified block position.
 * (Alternatives should be considered first in performance critical code.)
 */
int getBiomeAtPos(const LayerStack g, const Pos pos);

/* Finds a suitable pseudo-random location in the specified area.
 * This function is used to determine the positions of spawn and strongholds.
 * Warning: accurate, but slow!
 *
 * @mcversion        : Minecraft version (changed in: 1.7, 1.13)
 * @g                : generator layer stack
 * @cache            : biome buffer, set to NULL for temporary allocation
 * @centreX, centreZ : origin for the search
 * @range            : square 'radius' of the search
 * @isValid          : boolean array of valid biome ids (size = 256)
 * @seed             : seed used for the RNG
 *                     (initialise RNG using setSeed(&seed))
 * @passes           : number of valid biomes passed, set to NULL to ignore this
 */
Pos findBiomePosition(
        const int           mcversion,
        const LayerStack    g,
        int *               cache,
        const int           centerX,
        const int           centerZ,
        const int           range,
        const int *         isValid,
        int64_t *           seed,
        int *               passes
        );

/* Determines if the given area contains only biomes specified by 'biomeList'.
 * This function is used to determine the positions of villages, ocean monuments
 * and mansions.
 * Warning: accurate, but slow!
 *
 * @g          : generator layer stack
 * @cache      : biome buffer, set to NULL for temporary allocation
 * @posX, posZ : centre for the check
 * @radius     : 'radius' of the check area
 * @isValid    : boolean array of valid biome ids (size = 256)
 */
int areBiomesViable(
        const LayerStack    g,
        int *               cache,
        const int           posX,
        const int           posZ,
        const int           radius,
        const int *         isValid
        );

/* Finds the smallest radius (by square around the origin) at which all the
 * specified biomes are present. The input map is assumed to be a square of
 * side length 'sideLen'.
 *
 * @map             : square biome map to be tested
 * @sideLen         : side length of the square map (should be 2*radius+1)
 * @biomes          : list of biomes to check for
 * @bnum            : length of 'biomes'
 * @ignoreMutations : flag to count mutated biomes as their original form
 *
 * Return the radius on the square map that covers all biomes in the list.
 * If the map does not contain all the specified biomes, -1 is returned.
 */
int getBiomeRadius(
        const int *     map,
        const int       mapSide,
        const int *     biomes,
        const int       bnum,
        const int       ignoreMutations);



//==============================================================================
// Finding Strongholds and Spawn
//==============================================================================

/* Finds the block positions of the strongholds in the world. Note that the
 * number of strongholds was increased from 3 to 128 in MC 1.9.
 * Warning: Slow!
 *
 * @mcversion : Minecraft version (changed in 1.7, 1.9, 1.13)
 * @g         : generator layer stack [worldSeed should be applied before call!]
 * @cache     : biome buffer, set to NULL for temporary allocation
 * @locations : output block positions
 * @worldSeed : world seed of the generator
 * @maxSH     : Stop when this many strongholds have been found. A value of 0
 *              defaults to 3 for mcversion <= MC_1_8, and to 128 for >= MC_1_9.
 * @maxRadius : Stop searching if the radius exceeds this value in meters.
 *              Set this to 0 to ignore this condition.
 *
 * Returned is the number of strongholds found.
 */
int findStrongholds(
        const int           mcversion,
        LayerStack *        g,
        int *               cache,
        Pos *               locations,
        int64_t             worldSeed,
        int                 maxSH,
        const int           maxRadius
        );

/* Finds the spawn point in the world.
 * Warning: Slow, and may be inaccurate because the world spawn depends on
 * grass blocks!
 *
 * @mcversion : Minecraft version (changed in 1.7, 1.13)
 * @g         : generator layer stack [worldSeed should be applied before call!]
 * @cache     : biome buffer, set to NULL for temporary allocation
 * @worldSeed : world seed used for the generator
 */
Pos getSpawn(const int mcversion, LayerStack *g, int *cache, int64_t worldSeed);



//==============================================================================
// Validating Structure Positions
//==============================================================================

/************************ Biome Checks for Structures **************************
 *
 * Scattered features only do a simple check of the biome at the block position
 * of the structure origin (i.e. the north-west corner). Before 1.13 the type of
 * structure was determined by the biome, while in 1.13 the scattered feature
 * positions are calculated separately for each type. However, the biome
 * requirements remain the same:
 *
 *  Desert Pyramid: desert or desertHills
 *  Igloo         : icePlains or coldTaiga
 *  Jungle Pyramid: jungle or jungleHills
 *  Swamp Hut     : swampland
 *
 * Similarly, Ocean Ruins and Shipwrecks require any oceanic biome at their
 * block position.
 *
 * Villages, Monuments and Mansions on the other hand require a certain area to
 * be of a valid biome and the check is performed at a 1:4 scale instead of 1:1.
 * (Actually the area for villages has a radius zero, which means it is a simple
 * biome check at a 1:4 scale.)
 */


/* These functions perform a biome check at the specified block coordinates to
 * determine whether the corresponding structure would spawn there. You can get
 * the block positions using the appropriate getXXXPos() function.
 *
 * @g              : generator layer stack [set seed using applySeed()]
 * @cache          : biome buffer, set to NULL for temporary allocation
 * @blockX, blockZ : block coordinates
 *
 * In the case of isViableFeaturePos() the 'type' argument specifies the type of
 * scattered feature (as an enum) for which the check is performed.
 *
 * The return value is non-zero if the position is valid.
 */
int isViableFeaturePos(const int type, const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableVillagePos(const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableOceanMonumentPos(const LayerStack g, int *cache, const int blockX, const int blockZ);
int isViableMansionPos(const LayerStack g, int *cache, const int blockX, const int blockZ);



//==============================================================================
// Finding Properties of Structures
//==============================================================================

/* Initialises and returns a random seed used in the (16x16) chunk generation.
 * This random object is used for recursiveGenerate() which is responsible for
 * generating caves, ravines, mineshafts, and virtually all other structures.
 */
inline static int64_t chunkGenerateRnd(const int64_t worldSeed, const int chunkX, const int chunkZ)
{
    int64_t rnd = worldSeed;
    setSeed(&rnd);
    rnd = (nextLong(&rnd) * chunkX) ^ (nextLong(&rnd) * chunkZ) ^ worldSeed;
    setSeed(&rnd);
    return rnd;
}

/* Checks if the village in the given region would be infested by zombies.
 * (Minecraft 1.10+)
 */
int isZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ);

/* Checks if the village in the given region would generate as a baby zombie
 * village. (The fact that these exist could be regarded as a bug.)
 * (Minecraft 1.12+)
 */
int isBabyZombieVillage(const int mcversion, const int64_t worldSeed,
        const int regionX, const int regionZ);



//==============================================================================
// Seed Filters
//==============================================================================

/* Looks through the seeds in 'seedsIn' and copies those for which all
 * temperature categories are present in the 3x3 area centred on the specified
 * coordinates into 'seedsOut'. The map scale at this layer is 1:1024.
 *
 * @g            : generator layer stack, (NOTE: seed will be modified)
 * @cache        : biome buffer, set to NULL for temporary allocation
 * @seedsIn      : list of seeds to check
 * @seedsOut     : output buffer for the candidate seeds
 * @seedCnt      : number of seeds in 'seedsIn'
 * qcentX, centZ : search origin centre (in 1024 block units)
 *
 * Returns the number of found candidates.
 */
int64_t filterAllTempCats(
        LayerStack *        g,
        int *               cache,
        const int64_t *     seedsIn,
        int64_t *           seedsOut,
        const int64_t       seedCnt,
        const int           centX,
        const int           centZ
        );

/* Looks through the list of seeds in 'seedsIn' and copies those that have all
 * major overworld biomes in the specified area into 'seedsOut'. These checks
 * are done at a scale of 1:256.
 *
 * @g           : generator layer stack, (NOTE: seed will be modified)
 * @cache       : biome buffer, set to NULL for temporary allocation
 * @seedsIn     : list of seeds to check
 * @seedsOut    : output buffer for the candidate seeds
 * @seedCnt     : number of seeds in 'seedsIn'
 * @pX, pZ      : search starting coordinates (in 256 block units)
 * @sX, sZ      : size of the searching area (in 256 block units)
 *
 * Returns the number of seeds found.
 */
int64_t filterAllMajorBiomes(
        LayerStack *        g,
        int *               cache,
        const int64_t *     seedsIn,
        int64_t *           seedsOut,
        const int64_t       seedCnt,
        const int           pX,
        const int           pZ,
        const unsigned int  sX,
        const unsigned int  sZ
        );


#endif /* FINDERS_H_ */
