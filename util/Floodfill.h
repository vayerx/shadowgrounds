
#ifndef FLOODFILL_H
#define FLOODFILL_H

namespace util
{
	/**
	 *
	 * Interface for mapper objects that can be used with floodfill.
	 *
	 * @version 1.0, 27.8.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see FloodfillByteArrayMapper
	 * @see Floodfill
	 *
	 */

	class IFloodfillByteMapper
	{
	public:
		virtual ~IFloodfillByteMapper() {}
		virtual unsigned char getByte(int x, int y) = 0;
		virtual void setByte(int x, int y, unsigned char value) = 0;
	};


	/**
	 *
	 * A floodfill mapper for simple byte array (in format x + y * sizeX).
	 *
	 * @version 1.0, 27.8.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see IFloodfillByteMapper
	 * @see Floodfill
	 *
	 */

	class FloodfillByteArrayMapper : public IFloodfillByteMapper
	{
	public:
		FloodfillByteArrayMapper(int sizeX, int sizeY, unsigned char *buf);
		virtual ~FloodfillByteArrayMapper() {}
		
		virtual unsigned char getByte(int x, int y);
		virtual void setByte(int x, int y, unsigned char value); 
	private:
		unsigned char *buf;
		int sizeX;
		int sizeY;
	};

	/**
	 *
	 * A general purpose class for floodfilling.
	 *
	 * @version 1.0, 27.8.2002
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see IFloodfillByteMapper
	 *
	 */

	class Floodfill
	{
	public:
		static void fillWithByte(unsigned char fillByte, unsigned char areaByte, 
			int mapSizeX, int mapSizeY, unsigned char *map,
			bool areaIsBlocking = false, bool corners = false);

		static void fillWithByte(unsigned char fillByte, unsigned char areaByte, 
			int mapSizeX, int mapSizeY, IFloodfillByteMapper *mapper,
			bool areaIsBlocking = false, bool corners = false);
	};

}

#endif
