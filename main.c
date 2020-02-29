#include <stdio.h>
#include <errno.h>
#include <zconf.h>
#include <assert.h>
#include "layers.h"
#include "generator.h"
#include "finders.h"

static const unsigned int str2int(const char *str, int h) {
	return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ (unsigned int) (str[h]);
}

const char *versions[] = {"1.7", "1.8", "1.9", "1.10", "1.11", "1.12", "1.13", "1.13.2", "1.14", "1.15", "UNKNOWN"};

int euclideanDistance(int x1, int y1, int x2, int y2) {
	double dx = (x1 - x2);
	double dy = (y1 - y2);
	return (int) (dx * dx + dy * dy);
}

enum versions parse_version(char *s) {
	enum versions v;
	switch (str2int(s, 0)) {
		case 193357645 :
			v = MC_1_7;
			break;
		case 193366850:
			v = MC_1_8;
			break;
		case 193367875 :
			v = MC_1_9;
			break;
		case 2085846491 :
			v = MC_1_10;
			break;
		case 2085882490 :
			v = MC_1_11;
			break;
		case 2085918233 :
			v = MC_1_12;
			break;
		case 2085954232 :
			v = MC_1_13;
			break;
		case 3841915620:
			v = MC_1_13_2;
			break;
		case 2085703007:
			v = MC_1_14;
			break;
		case 2085739006:
			v = MC_1_15;
			break;
		default:
			v = MC_LEG;
			break;
	}
	return v;
}

void usage() {
	printf("Usage is ./WitchHutFinder [mcversion] [seed] [searchRange]? [filter]? \n"
		   "Valid [mcversion] are 1.7, 1.8, 1.9, 1.10, 1.11, 1.12, 1.13, 1.13.2, 1.14.\n"
		   "Valid [searchRange] (optional) is in blocks, default is 150000 which correspond to -150000 to 150000 on both X and Z.\n"
		   "Valid [filter] (optional) is either 2, 3 or 4 for respectively only outputting double, triple or quad witch huts as minimum.\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
	int mcversion = MC_1_12;
	int64_t seed = (int64_t) NULL;
	int searchRange = 300;
	char *endptr;
	int OFFSET=2;
	// Get the information to start the program
	if (argc > 2) {
		mcversion = parse_version(argv[1]);
		if (mcversion == MC_LEG) {
			usage();
		}
		errno = 0;
		seed = strtoll(argv[2], &endptr, 10);
		if ((errno == ERANGE && (seed == LLONG_MAX || seed == LLONG_MIN)) || (errno != 0 && seed == 0)) {
			fprintf(stderr, "Seed was not parsed correctly\n");
			usage();
		}
		if (endptr == argv[2]) {
			fprintf(stderr, "Seed contained letters\n");
			usage();
		}
	} else {
		usage();
	}
	if (argc > 3) {
		errno = 0;
		searchRange = (int) (strtoll(argv[3], &endptr, 10) / 32 / 16);
		if ((errno == ERANGE && (seed == LLONG_MAX || seed == LLONG_MIN)) || (errno != 0 && seed == 0)) {
			fprintf(stderr, "Search Range was not parsed correctly\n");
			usage();
		}
		if (endptr == argv[3]) {
			fprintf(stderr, "Search Range contained letters\n");
			usage();
		}
		if (searchRange<0){
			fprintf(stderr,"Search range should be positive");
			usage();
		}
	}
	if (argc > 4) {
		errno = 0;
		OFFSET = (int) (strtoll(argv[4], &endptr, 10));
		if ((errno == ERANGE && (seed == LLONG_MAX || seed == LLONG_MIN)) || (errno != 0 && seed == 0)) {
			fprintf(stderr, "Filter was not parsed correctly\n");
			usage();
		}
		if (endptr == argv[3]) {
			fprintf(stderr, "Filter contained letters\n");
			usage();
		}
		if (OFFSET<2 || OFFSET>4){
			fprintf(stderr,"Filter not in range");
			usage();
		}
	}
	OFFSET=OFFSET-4;
	printf("Using seed %ld and version %s\n", seed, versions[mcversion]);
	// Basic initialization
	StructureConfig featureConfig;
	initBiomes();
	LayerStack g;
	Pos qhpos[4];
	if (mcversion >= MC_1_13) {
		featureConfig = SWAMP_HUT_CONFIG;
		g = setupGeneratorMC113();
	} else {
		featureConfig = FEATURE_CONFIG;
		g = setupGeneratorMC17();
	}
	clock_t before = clock();
	int results[3] = {0, 0, 0};
	assert(seed != NULL);
	FILE *fp;

	fp = fopen("out.txt", "w+");
	fprintf(fp,"Using seed %ld and version %s\n", seed, versions[mcversion]);
	for (int regPosX = -searchRange; regPosX < searchRange; ++regPosX) {
		for (int regPosZ = -searchRange; regPosZ < searchRange; ++regPosZ) {
			qhpos[0] = getStructurePos(featureConfig, seed, 0 + regPosX, 0 + regPosZ);
			qhpos[1] = getStructurePos(featureConfig, seed, 0 + regPosX, 1 + regPosZ);
			qhpos[2] = getStructurePos(featureConfig, seed, 1 + regPosX, 0 + regPosZ);
			qhpos[3] = getStructurePos(featureConfig, seed, 1 + regPosX, 1 + regPosZ);
			//printf("(%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",qhpos[0].x,qhpos[0].z,qhpos[1].x,qhpos[1].z,qhpos[2].x,qhpos[2].z,qhpos[3].x,qhpos[3].z);

			int count = 0;
			applySeed(&g, seed);
			int correctPos[4] = {-1, -1, -1, -1};
			if (getBiomeAtPos(g, qhpos[0]) == swampland) {
				correctPos[count++] = 0;
			} else {
				if (count <= 0 + OFFSET) {
					continue;
				}
			}
			if (getBiomeAtPos(g, qhpos[1]) == swampland) {
				correctPos[count++] = 1;
			} else {
				if (count <= 1 + OFFSET) {
					continue;
				}
			}
			if (getBiomeAtPos(g, qhpos[2]) == swampland) {
				correctPos[count++] = 2;
			} else {
				if (count <= 2 + OFFSET) {
					continue;
				}
			}
			if (getBiomeAtPos(g, qhpos[3]) == swampland) {
				correctPos[count++] = 3;
			} else {
				if (count <= 3 + OFFSET) {
					continue;
				}
			}
			assert(count >= 4 + OFFSET);
			if (count >= 4 + OFFSET) {
				for (int j = 0; j < count - 1; ++j) {
					int maxi = count - j;
					int x = 0;
					int z = 0;
					for (int i = 0; i < maxi; ++i) {
						x += qhpos[correctPos[i]].x;
						z += qhpos[correctPos[i]].z;
					}
					x = (int) (x / (double) maxi);
					z = (int) (z / (double) maxi);
					int valid = 1;
					for (int i = 0; i < maxi; ++i) {
						if (euclideanDistance(qhpos[correctPos[i]].x, qhpos[correctPos[i]].z, x, z) > 16384)
							valid = 0;
					}
					if (valid && maxi>=OFFSET+4) {
						printf("CENTER for %d huts: %d,%d\n", maxi, x, z);
						fprintf(fp, "CENTER for %d huts: %d,%d\n", maxi, x, z);
						results[maxi - 2]++;
					}
				}
			}
		}
	}
	fclose(fp);
	clock_t difference = clock() - before;
	unsigned long msec = difference * 1000 / CLOCKS_PER_SEC;
	printf("Found %d double witch huts, %d triple witch huts, %d quad witch huts\n", results[0], results[1], results[2]);
	printf("Took %lu seconds %lu milliseconds for search range %d on seed %ld with generator %s\n", msec / 1000, msec % 1000, searchRange * 32 * 16, seed, versions[mcversion]);

}