
#ifndef DISTANCEFLOODFILL_H
#define DISTANCEFLOODFILL_H

namespace util
{
	/**
	 *
	 * Interface for objects that can monitor the progress of the
	 * distancefloodfilling.
	 *
	 * @version 1.0, 15.1.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see DistanceFloodfillByteArrayMapper
	 * @see DistanceFloodfill
	 *
	 */

	class IDistanceFloodfillMonitor
	{
	public:
		virtual ~IDistanceFloodfillMonitor() {}
		virtual void distanceFloodfillProgress(int line) = 0;
	};


	/**
	 *
	 * Interface for mapper objects that can be used with distance floodfill.
	 *
	 * @version 1.0, 14.1.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see DistanceFloodfillByteArrayMapper
	 * @see DistanceFloodfill
	 *
	 */

	class IDistanceFloodfillByteMapper
	{
	public:
		virtual ~IDistanceFloodfillByteMapper() {}
		virtual unsigned char getByte(int x, int y) = 0;
		virtual void setByte(int x, int y, unsigned char value) = 0;
	};


	/**
	 *
	 * A distance floodfill mapper for simple byte array (in format x + y * sizeX).
	 *
	 * @version 1.0, 14.1.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see IDistanceFloodfillByteMapper
	 * @see DistanceFloodfill
	 *
	 */

	class DistanceFloodfillByteArrayMapper : public IDistanceFloodfillByteMapper
	{
	public:
		DistanceFloodfillByteArrayMapper(int sizeX, int sizeY, unsigned char *buf);
	
		virtual ~DistanceFloodfillByteArrayMapper() {}
		
		virtual unsigned char getByte(int x, int y);
		virtual void setByte(int x, int y, unsigned char value); 
	private:
		unsigned char *buf;
		int sizeX;
		int sizeY;
	};

	/**
	 *
	 * A general purpose class for distance floodfilling.
	 *
	 * Takes two maps, one of which defines the obstacles and the
	 * other is used for marking ranges to nearest targets.
	 * The other map should be initially set to 255 except for those
	 * blocks that are the targets. The targets should be set to 0.
	 * Then the floodfiller will fill the 255 blocks with proper
	 * values that are between 1-255.
	 * 
	 * The resulting range map can be easily used for pathfinding to
	 * the target(s).
	 *
	 * @version 1.0, 14.1.2003
	 * @author Jukka Kokkonen <jukka@frozenbyte.com>
	 * @see IDistanceFloodfillByteMapper
	 *
	 */

	class DistanceFloodfill
	{
	public:
		static void fillRanges(unsigned char areaByte, 
			int mapSizeX, int mapSizeY, unsigned char *map, 
			unsigned char *rangeMap, 
			bool areaIsBlocking = false, bool corners = false,
			IDistanceFloodfillMonitor *monitor = 0);

		static void fillRanges(unsigned char areaByte, 
			int mapSizeX, int mapSizeY, IDistanceFloodfillByteMapper *mapper,
			IDistanceFloodfillByteMapper *rangeMapper,
			bool areaIsBlocking = false, bool corners = false,
			IDistanceFloodfillMonitor *monitor = 0);
	};

}

#endif

