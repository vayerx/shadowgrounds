
#ifndef MATERIALS_H
#define MATERIALS_H

#define MATERIAL_AMOUNT 16

#define MATERIAL_SAND 0
#define MATERIAL_ROCK 1
#define MATERIAL_CONCRETE 2
#define MATERIAL_METAL_HARD 3
#define MATERIAL_METAL_TIN 4
#define MATERIAL_WOOD 5
#define MATERIAL_GLASS 6
#define MATERIAL_PLASTIC 7
#define MATERIAL_LEAVES 8
#define MATERIAL_LIQUID 9
#define MATERIAL_METAL_GRATE 10
#define MATERIAL_SOIL 11
#define MATERIAL_GRASS 12
#define MATERIAL_SNOW_SHALLOW 13
#define MATERIAL_SNOW_DEEP 14
#define MATERIAL_RESERVED_15 15

// note: should match the material amount in areamask
#ifdef PROJECT_SHADOWGROUNDS
#define MATERIAL_PALETTE_AMOUNT 8
#else
#define MATERIAL_PALETTE_AMOUNT 16
#endif

namespace game
{
  extern const char *materialName[MATERIAL_AMOUNT];

  extern int materialsInUse[MATERIAL_PALETTE_AMOUNT];

	extern void changeMaterialPalette(int paletteNum, int material);

	extern int getMaterialByPalette(int paletteNum);

}

#endif

