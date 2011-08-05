
#ifndef RLEBLOCKMAP_H
#define RLEBLOCKMAP_H

namespace util
{
	template <class T> class RLEBlockMap
	{
		public:
			RLEBlockMap(int cacheSize)

			void create(T *unpackedDataBuffer, int sizeX, int sizeY);

			void loadFromPacked(void *packedData, int sizeX, int sizeY);

			void saveToPacked(void *packedData);

			T get(int x, int y);

			//void set(int x, int y, T value);
			//void commit(); // or maybe autocommit?

		private:
			T **packedBlocks;
			T **unpackedBlocks;
			int cacheSize;
			int nextSlot;
			//bool *dirtySlot;

			int sizeX;
			int sizeY;
	};
}

typedef RLEBlockMap<unsigned char> RLEBlockMap8Bit;
typedef RLEBlockMap<unsigned short> RLEBlockMap16Bit;

#endif


