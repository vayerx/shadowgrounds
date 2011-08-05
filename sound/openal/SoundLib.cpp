
#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(LINUX)
#define Sleep usleep
#endif

#include <vector>

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread.hpp>

#if defined _MSC_VER || defined __APPLE__
#include <al.h>
#include <alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <vorbis/vorbisfile.h>


#include "filesystem/file_package_manager.h"
#include "storm/include/c2_common.h"
#include "storm/include/c2_vectors.h"
#include "storm/include/istorm3d_streambuffer.h"
#include "sound/SoundLib.h"
#include "sound/WaveReader.h"
#include "system/Miscellaneous.h"
#include "system/Logger.h"
#include "system/Timer.h"

using namespace frozenbyte;


#define alErrors() alErrors_(__FILE__, __LINE__)

static bool alErrors_(const char *file, int line)
{
	ALenum error = alGetError();
	if (error != AL_NO_ERROR)
	{
		std::string desc;
		switch(error)
		{
			case AL_INVALID_NAME:
				desc = "Invalid Name paramater passed to AL call.";
				break;
			case AL_ILLEGAL_ENUM:
				desc = "Invalid parameter passed to AL call.";
				break;
			case AL_INVALID_VALUE:
				desc = "Invalid enum parameter value.";
				break;
			case AL_ILLEGAL_COMMAND:
				desc = "Illegal call.";
				break;
			case AL_OUT_OF_MEMORY:
				desc = "Out of memory.";
				break;
			default:
				desc = "Unknown AL error.";
				break;
		}
		LOG_ERROR((strPrintf("OpenAL error at %s:%d : ", file, line) + desc).c_str());
		return true;
	}

	return false;
}


static std::string errorString(int code)
{
    switch(code)
    {
        case OV_EREAD:
            return std::string("Read from media.");
        case OV_ENOTVORBIS:
            return std::string("Not Vorbis data.");
        case OV_EVERSION:
            return std::string("Vorbis version mismatch.");
        case OV_EBADHEADER:
            return std::string("Invalid Vorbis header.");
        case OV_EFAULT:
            return std::string("Internal logic fault (bug or heap/stack corruption.");
        default:
            return std::string("Unknown Ogg error.");
    }
}


// how many openal buffers per stream
static unsigned int streamBuffers = 4;

// size of one buffer
static unsigned int bufferSize = 2*131072;


namespace sfx
{
	float streamVolume = 1.0f;

	class SoundSource;

    struct SoundBufferImpl : public boost::noncopyable
	{
		ALuint buffer;

		friend class SoundSource;

	public:
		SoundBufferImpl()
		: buffer(AL_NONE)
		{
			alErrors();
			alGenBuffers(1, &buffer);
			alErrors();
		}

		~SoundBufferImpl()
		{
			alErrors();
			if (buffer != AL_NONE)
			{
				alDeleteBuffers(1, &buffer);
				alErrors();

				buffer = AL_NONE;
			}
		}

	};


	class SoundBuffer
	{
		friend class SoundSource;

		boost::shared_ptr<SoundBufferImpl> impl;

	public:

		SoundBuffer()
		: impl(new SoundBufferImpl)
		{
		}

		void bufferData(ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
		{
			assert(impl);
			assert(impl->buffer != AL_NONE);

			alBufferData(impl->buffer, format, data, size, freq);
			alErrors();
		}
	};


	class SoundSource : public boost::noncopyable
	{
		ALuint source;

		// map from openal buffer number to SoundBuffer object
		// ALuint == SoundBuffer.buffer
		typedef std::map<ALuint, SoundBuffer> BufferMap;

		BufferMap queuedBuffers;

	public:
		SoundSource()
		: source(AL_NONE)
		{
			alErrors();
			alGenSources(1, &source);
			alErrors();
		}

		~SoundSource()
		{
			alErrors();
			if (source != AL_NONE)
			{
				alDeleteSources(1, &source);
				alErrors();
				source = AL_NONE;
			}
		}

		// apparently alSourceQueueBuffers will return error if there are
		// already queued buffers
		// so need to buffer all at once
		void queueBuffers(const std::vector<SoundBuffer> &buffers)
		{
			std::vector<ALuint> rawBuffers;

			BOOST_FOREACH (const SoundBuffer &buffer, buffers)
			{
				BufferMap::iterator it = queuedBuffers.find(buffer.impl->buffer);
				if (it != queuedBuffers.end())
				{
					LOG_ERROR("OpenAL: queueing buffer which is already queued on this source");
				}

				queuedBuffers.insert(std::make_pair(buffer.impl->buffer, buffer));
				rawBuffers.push_back(buffer.impl->buffer);
			}

			alSourceQueueBuffers(source, rawBuffers.size(), &rawBuffers[0]);
			alErrors();
		}

		void play()
		{
			alSourcePlay(source);
			alErrors();
		}

		void pause()
		{
			alSourcePause(source);
			alErrors();
		}

		void stop()
		{
			alSourceStop(source);
			alErrors();
		}

		ALenum getState()
		{
			ALenum state = AL_STOPPED;

			alGetSourcei(source, AL_SOURCE_STATE, &state);
			alErrors();

			return state;
		}

		unsigned int getNumProcessedBuffers()
		{
			int processed = 0;

			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
			alErrors();

			return processed;
		}

		bool hasProcessedBuffers() const
		{
			int processed = 0;

			alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
			alErrors();

			return (processed != 0);
		}

		SoundBuffer getProcessedBuffer()
		{
			assert(hasProcessedBuffers());

            // get the raw buffer
			ALuint rawBuffer = AL_NONE;
			alSourceUnqueueBuffers(source, 1, &rawBuffer);
			alErrors();

			// look up the SoundBuffer object
			if (rawBuffer == AL_NONE)
			{
				LOG_ERROR("OpenAL: alSourceUnqueueBuffers returned AL_NONE");
			} else {
				BufferMap::iterator it = queuedBuffers.find(rawBuffer);
				if (it == queuedBuffers.end())
				{
					LOG_ERROR("OpenAL: unqueued buffer not in queuedBuffers");
				} else {
					SoundBuffer buffer = (*it).second;
					queuedBuffers.erase(it);

					return buffer;
				}
			}

			return SoundBuffer();
		}

		unsigned int getNumQueuedBuffers()
		{
			int queued = 0;

			alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
			alErrors();

			return queued;
		}

		operator ALuint() const
		{
			return source;
		}

	};

	struct Sound::Data : public boost::noncopyable
	{
		friend class Sound;
		friend class SoundLib;
		friend class SoundFileProcesser;

		std::string filename;

		int length;
		int freq;

		bool fileNotFound;
		ALuint sample;


		Data(const char *file, int flags);

		~Data()
		{
			alErrors();

			if (sample != AL_NONE)
			{
				alDeleteBuffers(1, &sample);
				alErrors();
				sample = AL_NONE;
			}
		}
	};


struct OggSoundData : public boost::noncopyable {
	const char *data;  // not owned

	unsigned int currentPos;
	unsigned int dataSize;


	OggSoundData(const char *data_, unsigned int dataSize_)
	: data(data_)
	, currentPos(0)
	, dataSize(dataSize_)
	{
	}
};


// read s * n bytes to ptr
static size_t soundData_read(void *ptr, size_t s, size_t n, void *data_)
{
	if (s * n == 0)  // if s or n is 0
		return 0;

	OggSoundData *data = (OggSoundData *) data_;

	// how many bytes max can be read
	size_t maxReadBytes = data->dataSize - data->currentPos;

	// how many bytes we're going to read
	size_t readBytes = std::min(n * s, maxReadBytes);

	memcpy(ptr, data->data + data->currentPos, readBytes);
	data->currentPos += readBytes;

	return (readBytes / s);  // fread is a stupid interface
}


static int soundData_seek(void *data_, ogg_int64_t offset, int whence)
{
	OggSoundData *data = (OggSoundData *) data_;

	switch (whence)
	{
	case SEEK_SET:
		if (offset > data->dataSize)
			return -1;

		data->currentPos = offset;
		break;

	case SEEK_CUR:
		if (data->currentPos + offset > data->dataSize)
			return -1;

		data->currentPos = data->currentPos + offset;
		break;

	case SEEK_END:
		if (offset > data->dataSize)
			return -1;

		data->currentPos = data->dataSize - offset;
		break;
	}

	return 0;
}


static long soundData_tell(void *data_)
{
	OggSoundData *data = (OggSoundData *) data_;
	return data->currentPos;
}


static const ov_callbacks soundCallbacks =
{
	  soundData_read
	, soundData_seek
	, NULL  // close
	, soundData_tell
};


Sound::Data::Data(const char *file, int flags)
: filename(file)
, length(0)
, freq(0)
, fileNotFound(false)
, sample(AL_NONE)
{
	// ogg or wav?
	bool isOgg = (filename.find(".ogg") != std::string::npos);

	std::vector<char> buffer;
	ALenum format;

	if (isOgg)
	{
		filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(filename);
		if(fileStream.isEof())
			return;

		int dataSize = fileStream.getSize();
		boost::scoped_array<char> audioData(new char[dataSize]);
		audioData.reset(new char[dataSize]);
		fileStream.read(audioData.get(), fileStream.getSize());

		OggVorbis_File oggStream;
		memset(&oggStream, 0, sizeof(OggVorbis_File));

		OggSoundData oggData(audioData.get(), dataSize);

		int result = ov_open_callbacks(&oggData, &oggStream, NULL, 0, soundCallbacks);

		if (result < 0)
		{
			LOG_ERROR(strPrintf("Could not open Ogg stream: %s %s\n", filename.c_str(), errorString(result).c_str()).c_str());
			return;
		}

		vorbis_info *vorbisInfo = ov_info(&oggStream, -1);
		if (vorbisInfo == NULL)
		{
			LOG_ERROR("ov_info failed");
			ov_clear(&oggStream);
			return;
		}

		if(vorbisInfo->channels == 1)
			format = AL_FORMAT_MONO16;
		else
			format = AL_FORMAT_STEREO16;

		unsigned int bufPos = 0;
		unsigned int bufRemain = 16384; // how much free space in buffer
		int section = 0;

		buffer.resize(bufRemain);
		while (true)
		{
			int ret = ov_read(&oggStream, &buffer[bufPos], bufRemain, 0, 2, 1, &section);
			if (ret <= 0)
			{
				length = 1000 * bufPos / (vorbisInfo->channels * vorbisInfo->rate * 2); // 16-bit samples = 2 bytes

				// EOF
				// remove trailing crap
				buffer.resize(bufPos);
				break;
			}
			if (ret > 0)
			{
				bufPos += ret;
				bufRemain -= ret;

				if (bufRemain == 0)
				{
					// ran out of space in buffer, enlarge
					buffer.resize(2 * bufPos);

					bufRemain = bufPos;
				}
			}

			// just skip errors... FIXME

		}

		freq = vorbisInfo->rate;
		ov_clear(&oggStream);
	} else {
		// is not ogg
		WaveReader reader(filename);

		if (reader)
		{
			reader.decodeAll(buffer);

			freq = reader.getFreq();
			unsigned int numChannels = reader.getChannels();
			unsigned int bits = reader.getBits();
			length = 1000 * buffer.size() / (freq * numChannels * bits / 8);

			if (numChannels == 1)
			{
				if (bits == 16)
				{
					format = AL_FORMAT_MONO16;
				} else {
					format = AL_FORMAT_MONO8;
				}
			} else {
				if (bits == 16)
				{
					format = AL_FORMAT_STEREO16;
				} else {
					format = AL_FORMAT_STEREO8;
				}
			}
		}
	}

	if (length == 0)
	{
		LOG_ERROR(strPrintf("Loading sound failed %s", filename.c_str()).c_str());
	} else {
		alGenBuffers(1, &sample);
		alErrors();

		alBufferData(sample, format, &buffer[0], buffer.size(), freq);
		alErrors();
	}
}


Sound::Sound(Data *data_)
: data(data_)
{
}


	Sound::~Sound()
	{
	}



	int Sound::getLength() const
	{
		return data->length;
	}


	const std::string &Sound::getFileName() const
	{
		return data->filename;
	}

	struct StreamProcesser;

	struct SoundStream::Data : public boost::noncopyable
	{
		friend class SoundStream;
		friend class SoundLib;
		friend struct StreamProcesser;

		std::string filename;  // of the wav

		int type; // SoundMixer::SoundStreamType
		int id;  // backgroundFileHack thing
		float baseVolume;
		float volume;

		bool fileNotFound;
		bool playing;
		bool loop;
		bool finished;  // ogg stream has finished
						// does not mean that all of it has been played yet

		OggVorbis_File  oggStream;
		vorbis_info*    vorbisInfo;

		boost::shared_array<char> streamData; // contains raw ogg data
		long currentPos;
		size_t dataSize;

		bool valid;

		boost::scoped_array<SoundBuffer> buffers;
		SoundSource &source;
		ALenum format;

		SoundLib *soundLib;

		Data(SoundSource &source_, std::string filename_)
		: type(0)
		, id(-1)
		, baseVolume(1.f)
		, volume(1.f)
		, fileNotFound(false)
		, playing(false)
		, loop(false)
		, finished(false)
		, vorbisInfo(NULL)
		, currentPos(0)
		, dataSize(0)
		, valid(false)
		, source(source_)
		, format(AL_NONE)
		, soundLib(NULL)
		{
			buffers.reset(new SoundBuffer[streamBuffers]);

			filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(filename_);
			if(fileStream.isEof())
				return;

			dataSize = fileStream.getSize();
			streamData.reset(new char[dataSize]);
			fileStream.read(streamData.get(), fileStream.getSize());

			// we got the data, open the file

			int result;
			if((result = ov_open_callbacks(this, &oggStream, NULL, 0, SoundStream::Data::streamProcesserCallbacks)) < 0)
			{
				LOG_ERROR(strPrintf("Could not open Ogg stream: %s %s\n", filename.c_str(), errorString(result).c_str()).c_str());
				return;
			}
			valid = true;

			vorbisInfo = ov_info(&oggStream, -1);

			if(vorbisInfo->channels == 1)
				format = AL_FORMAT_MONO16;
			else
				format = AL_FORMAT_STEREO16;

			// queue the initial data
			std::vector<SoundBuffer> buffersToQueue;
			for (unsigned int i = 0; i < streamBuffers; i++)
			{
				bool active = stream(buffers[i]);
				if (active)
				{
					buffersToQueue.push_back(buffers[i]);
				}
			}
			source.queueBuffers(buffersToQueue);

			if (playing)
			{
				ALenum state = source.getState();

				if(state != AL_PLAYING)
				{
					source.play();
				}
			}

		}


		~Data();


		bool update();

		bool stream(SoundBuffer buffer);


		static const ov_callbacks streamProcesserCallbacks;

        // read s * n bytes to ptr
		static size_t soundStream_read(void *ptr, size_t s, size_t n, void *streamData)
		{
			if (s * n == 0)  // if s or n is 0
				return 0;

			Data *data = (Data *) streamData;

			// how many bytes max can be read
			size_t maxReadBytes = data->dataSize - data->currentPos;

			// how many bytes we're going to read
			size_t readBytes = std::min(n * s, maxReadBytes);

			memcpy(ptr, data->streamData.get() + data->currentPos, readBytes);
			data->currentPos += readBytes;

			return (readBytes / s);  // fread is a stupid interface
		}

		static int soundStream_seek(void *streamData, ogg_int64_t offset, int whence)
		{
			Data *data = (Data *) streamData;

			switch (whence)
			{
			case SEEK_SET:
				if (offset > data->dataSize)
					return -1;

				data->currentPos = offset;
				break;

			case SEEK_CUR:
				if (data->currentPos + offset > data->dataSize)
					return -1;

				data->currentPos = data->currentPos + offset;
				break;

			case SEEK_END:
				if (offset > data->dataSize)
					return -1;

				data->currentPos = data->dataSize - offset;
				break;
			}

			return 0;
		}

		static long soundStream_tell(void *streamData)
		{
			return ((Data *) streamData)->currentPos;
		}

	};


	// ptr is type SoundStream::Data
	const ov_callbacks SoundStream::Data::streamProcesserCallbacks =
	{
		&soundStream_read
		, &soundStream_seek
		, NULL  // no explicit close function
		, &soundStream_tell
	};



	static const int BUFFER_SAMPLES = 2000;
	static const uint64_t BUFFER_TIME_ADD = BUFFER_SAMPLES * 10000 / 2;
	class StormStream : public IStorm3D_Stream, public boost::noncopyable
	{
		struct SampleBuffer
		{
			uint64_t start;
			uint64_t duration;
			std::vector<char> buffer;

			SampleBuffer()
			:	start(0),
				duration(0)
			{
			}
		};

		typedef std::list<SampleBuffer> BufferList;

		bool stereo;
		int frequency;
		int bits;

		SoundSource &source;
		SoundLib *soundLib;

		std::vector<SoundBuffer> availableBuffers;
		int channel;

		BufferList buffers;
		int position;

		mutable boost::mutex lock;

		int updateTime;
		int updates;
		int sampleSize;
		bool active;

		int time;
		ALenum format;

	public:
		StormStream(bool stereo_, int frequency_, int bits_, float volume, SoundSource &source_, SoundLib *soundLib_)
		:	stereo(stereo_)
		,	frequency(frequency_)
		,	bits(bits_)
		,	source(source_)
		,	soundLib(soundLib_)
		,	channel(-1)
		,	position(0)
		,	updateTime(0)
		,	sampleSize(0)
		,	active(false)
		,	time(0)
		,	format(AL_NONE)
		{
			if(stereo)
				sampleSize = 2 * (bits / 8);
			else
				sampleSize = (bits / 8);

			updateTime = BUFFER_SAMPLES * 1000 / frequency;

			if(stereo)
			{
				if (bits == 16)
				{
					format = AL_FORMAT_STEREO16;
				} else {
					format = AL_FORMAT_STEREO8;
				}
			} else {  // mono
				if (bits == 16)
				{
					format = AL_FORMAT_MONO16;
				} else {
					format = AL_FORMAT_MONO8;
				}
			}

			for (unsigned int i = 0; i < streamBuffers; i++)
			{
				availableBuffers.push_back(SoundBuffer());
			}
		}

		~StormStream();

		void activate()
		{
			if(!active)
			{
				updates = 0;
				time = Timer::getCurrentTime();
				active = true;
			}
		}

		void deactivate()
		{
			boost::mutex::scoped_lock locked(lock);

			active = false;
			buffers.clear();
			position = 0;

			source.stop();
		}

		void addSample(const char *buffer_, int length, uint64_t start, uint64_t duration)
		{
			boost::mutex::scoped_lock locked(lock);

			buffers.push_back(SampleBuffer());

			SampleBuffer &sample = buffers.back();
			sample.start = start;
			sample.duration = duration;
			sample.buffer.resize(length);

			memcpy(&sample.buffer[0], buffer_, length);
		}

		uint64_t getCurrentTime() const
		{
			static const int64_t delta = (200 * 10000);

			if(!active)
				return 0;

			//int ms = (timeGetTime() - time);
			//__int64 result = __int64(ms) * 10000;
			//return result;

			int64_t time = int64_t(updates * BUFFER_SAMPLES) * 1000 * 10000 / frequency;

			if(time < delta)
				return 0;
			return time - delta;
		}

		void update()
		{
			boost::mutex::scoped_lock locked(lock);

			// unqueue buffers which have completed
			while (source.hasProcessedBuffers())
			{
				availableBuffers.push_back(source.getProcessedBuffer());
			}

			// queue data until either no buffers left
			// or no data left
			std::vector<SoundBuffer> buffersToQueue;

			while (! (availableBuffers.empty() || buffers.empty() ) )
			{
				int len = 44100;
				boost::scoped_array<char> scopedBuff(new char[len]);
				char *buff = scopedBuff.get();

				// copypasta from SoundLib_StreamBuilder.h
				// no idea what it really does
				++updates;
				int written = 0;
				while(!buffers.empty() && written < len)
				{
					BufferList::iterator first = buffers.begin();
					const std::vector<char> &buffer = first->buffer;

					int size = buffer.size();
					int start = position;
					int end = position + (len - written);
					if(end > size)
						end = size;

					if(start < size - 1 && end <= size)
					{
						int amount = end - start;
						memcpy((char *)buff + written, &buffer[start], amount);
						written += amount;
						position += amount;
					}

					if(written < len)
					{
						position = 0;
						buffers.erase(first);
					}
				}

				// put it in openal buffer
				SoundBuffer alBuffer = availableBuffers.back();
				availableBuffers.pop_back();

				alBuffer.bufferData(format, buff, written, frequency);

				// queue
				buffersToQueue.push_back(alBuffer);
			}
			source.queueBuffers(buffersToQueue);

			// call play if stream stopped but should be playing
			if (active)
			{
				ALenum state = source.getState();
				if (state != AL_PLAYING)
				{
					source.play();
				}
			}
		}
	};

	class StormStreamBuilder : public IStorm3D_StreamBuilder, public boost::noncopyable
	{
	public:
		bool stereo;
		int frequency;
		int bits;

		SoundLib *soundLib;

		std::vector<boost::shared_ptr<StormStream> > streams;

		StormStreamBuilder(SoundLib *soundLib_)
		:	stereo(true)
		,	frequency(22050)
		,	bits(16)
		,	soundLib(soundLib_)
		{
		}

		~StormStreamBuilder()
		{
		}

		void setStereo(bool data)
		{
			stereo = data;
		}

		void setFrequency(int data)
		{
			frequency = data;
		}

		void setBits(int data)
		{
			bits = data;
		}

		void update()
		{
			// iterator::erase can invalidate iterators
			// so need to delay delete
			bool deletedSomeone = false;

			BOOST_FOREACH (boost::shared_ptr<StormStream> &stream, streams)
			{
				// if it's unique, users have already forgotten it
				if (stream.unique())
				{
					// delete it
					stream.reset();
					deletedSomeone = true;
				} else {
					// otherwise update it
					stream->update();
				}
			}

			if (deletedSomeone)
			{
				// using goto
				// would really like tail recursion
			deleteStart:
				std::vector<boost::shared_ptr<StormStream> >::iterator deletedStream = std::find(streams.begin(), streams.end(), boost::shared_ptr<StormStream>());
				if (deletedStream != streams.end())
				{
					// remove it from list
					streams.erase(deletedStream);

					// anyone else?
					goto deleteStart;
				}
			}
		}

		boost::shared_ptr<IStorm3D_Stream> getStream()
		{
			boost::shared_ptr<StormStream> stream = boost::static_pointer_cast<StormStream>(soundLib->createStormStream(stereo, frequency, bits, streamVolume));
			streams.push_back(stream);
			return boost::static_pointer_cast<IStorm3D_Stream>(stream);
		}
	};


	struct SoundLib::Data : public boost::noncopyable
	{
		friend class SoundLib;
		friend class SoundStream;

		bool initialized;
		bool useHardware;
		bool useEax;
		int mixrate;
		int softwareChannels;
		int minHardwareChannels;
		int maxHardwareChannels;
		int speakerType;

		float frequencyFactor;

		StormStreamBuilder streamBuilder;

		struct Channel : public boost::noncopyable
		{
			SoundSource source;  // owned by this Channel
			Sound *sound;  // NOT owned
			int priority;
			bool playing;  // the sample on this channel is supposed to be playing
			bool looping;
			bool loanedToStream;  // this channel has been "loaned" to a stream

			Channel()
			: sound(NULL)
			, priority(-1)
			, playing(false)
			, looping(false)
			, loanedToStream(false)
			{
			}

			~Channel()
			{
				sound = NULL;
				playing = false;
				priority = -1;
			}
		};

		unsigned int numchannels;

		boost::scoped_array<Channel> channels;

		std::vector<SoundStream *> streams;

	public:
		Data(SoundLib *soundLib_)
		: initialized(false)
		, useHardware(false)
		, useEax(false)
		, mixrate(0)
		, softwareChannels(0)
		, minHardwareChannels(0)
		, maxHardwareChannels(0)
		, speakerType(0)
		, frequencyFactor(0.0f)
		, streamBuilder(soundLib_)
		, numchannels(0)
		{
		}


		~Data();


		int findFreeChannel(int priority)
		{
			// find a free channel
			int freeChannel = -1; // first free channel

			// a non-free channel which has lower priority than us
			// used in case no free channel found
			int priorityFreeChannel = -1;

			for (unsigned int i = 0; i < numchannels; i++)
			{
				if (channels[i].loanedToStream)
				{
					// never steal from a stream
					continue;
				}

				if (channels[i].sound == NULL)
				{
					// it's free, use it
					freeChannel = i;
					// no need to continue
					break;
				} else if (channels[i].priority < priority)
				{
					// it's not free, check if it can be used for priority-based freeing

					// if no priority-based selection made, use this
					if (priorityFreeChannel < 0
						// or if it has a lower priority than current selection
					   || channels[i].priority < channels[priorityFreeChannel].priority)
					{
						priorityFreeChannel = i;
					}

					// keep going in case we find a better alternative
				}
			}

			if (freeChannel < 0)
			{
				if (priorityFreeChannel < 0)
				{
					LOG_ERROR("SoundLib impossible: neither free channel nor priority-based free channel");
				}

				freeChannel = priorityFreeChannel;
			}

			return freeChannel;
		}
	};


	StormStream::~StormStream()
	{
		// release the channel back to other use
		for (unsigned int i = 0; i < soundLib->data->numchannels; i++)
		{
			SoundLib::Data::Channel &channel = soundLib->data->channels[i];
			if (channel.source == source) {
				// found "our" channel
				channel.loanedToStream = false;
				break;
			}
		}

		// unqueue possible remaining queued buffesr
		int queued = source.getNumQueuedBuffers();

		while(queued--)
		{
			ALuint buffer;

			alSourceUnqueueBuffers(source, 1, &buffer);
			alErrors();
		}
	}


	// returns true if queued new data
	bool SoundStream::Data::stream(SoundBuffer buffer)
	{
		alErrors();

		if (!valid)
			return false;

		boost::scoped_array<char> pcm(new char[bufferSize]);
		unsigned int size = 0;
		int  section;
		int  result;

		while(size < bufferSize)
		{
			result = ov_read(&oggStream, pcm.get() + size, bufferSize - size, 0, 2, 1, &section);

			if(result > 0)
			{
				size += result;
			}
			else
			{
				if(result < 0)
				{
					LOG_ERROR(strPrintf("SoundStream::Data::stream error: %s\n", errorString(result).c_str()).c_str());
					return false;
				}
				else
					break;
			}
		}
		
		if(size == 0)
		{
			// stream ended
			if (loop)
			{
				// it appears that this is never used
				// someone else takes care of this after stream has ended

				// seek to beginning
				int result = ov_raw_seek(&oggStream, 0);
				if (result != 0) {
					LOG_ERROR(strPrintf("SoundStream: ov_raw_seek failed: %d", result).c_str() );
				}

				// go back to trying to stream it
				return stream(buffer);
			}

			finished = true;
			return false;
		}

		// LOG_DEBUG(strPrintf("alBufferData size = %d, rate = %ld, pcm = %d",size,vorbisInfo->rate,reinterpret_cast<unsigned int>(pcm.get())));
		buffer.bufferData(format, pcm.get(), size, vorbisInfo->rate);

		return true;
	}


	bool SoundStream::Data::update()
	{
		// update is only called if stream is supposed to be playing

		alErrors();

		if (!valid)
			return false;

		// finished but playing
        // -> ogg stream has finished but there's still queued stuff
		if (finished)
		{
			ALenum state = source.getState();

			if(state != AL_PLAYING)  // if stream has stopped, it really has stopped
			{
				playing = false;
                return false;
			}

			return true;
		}

		bool hasNewData = false;
		std::vector<SoundBuffer> buffersToQueue;
		while (source.hasProcessedBuffers())
		{
			SoundBuffer buffer = source.getProcessedBuffer();

			bool active = stream(buffer);

			if (active)
			{
				buffersToQueue.push_back(buffer);
			}

			hasNewData = hasNewData || active;
		}

		if (!buffersToQueue.empty())
		{
			source.queueBuffers(buffersToQueue);

			// restart if stopped
			ALenum state = source.getState();
			if(state != AL_PLAYING)
			{
				source.play();
			}
		}

		return hasNewData;
	}



SoundStream::SoundStream(Data *data_)
: data(data_)
{
}


SoundStream::~SoundStream()
{
}


	SoundStream::Data::~Data()
	{
		// remove ourselves from SoundLib list of playing streams
		// need some trickery because we are Data and playing streams
		// stores SoundStreams
		SoundStream *streamToRemove = NULL;
		BOOST_FOREACH(SoundStream *stream, soundLib->data->streams)
		{
			if (stream->data.get() == this)
			{
				streamToRemove = stream;
				break;
			}
		}

		if (streamToRemove != NULL)
		{
			std::vector<SoundStream *>::iterator it = find(soundLib->data->streams.begin()
												 , soundLib->data->streams.end()
												 , streamToRemove);
			if (it != soundLib->data->streams.end())
			{
				soundLib->data->streams.erase(it);
			}
		}

		// release the channel back to other use
		for (unsigned int i = 0; i < soundLib->data->numchannels; i++)
		{
			SoundLib::Data::Channel &channel = soundLib->data->channels[i];
			if (channel.source == source) {
				// found "our" channel
				channel.loanedToStream = false;
				break;
			}
		}

		{
			alErrors();

			if (valid)
			{
				source.stop();
				{
					int queued = source.getNumQueuedBuffers();

					while(queued--)
					{
						ALuint buffer;

						alSourceUnqueueBuffers(source, 1, &buffer);
						alErrors();
					}
				}

				ov_clear(&oggStream);
				memset(&oggStream, 0, sizeof(OggVorbis_File));
			}

		}

	}


	void SoundStream::setBaseVolume(float value)
	{
		data->baseVolume = value;

		float vol = data->baseVolume * data->volume;
		if(vol < 0.0)
			vol = 0.0;
		if(vol > 1.0)
			vol = 1.0;

		alSourcef(data->source, AL_GAIN, vol);
		alErrors();
	}


	void SoundStream::setVolume(float value)
	{
		data->volume = value;

		float vol = data->baseVolume * data->volume;
		if(vol < 0.0)
			vol = 0.0;
		if(vol > 1.0)
			vol = 1.0;

		alSourcef(data->source, AL_GAIN, vol);
		alErrors();
	}


	void SoundStream::setPanning(float value) {}


	// this appears to be never used...
	void SoundStream::setLooping(bool loop)
	{
		data->loop = loop;
	}


	void SoundStream::play()
	{
		data->playing = true;

		if (data->id < 0) // has it loaded yet?
		{
			data->source.play();
		}
	}


	void SoundStream::stop()
	{
		data->playing = false;
		data->source.stop();
		ov_raw_seek(&data->oggStream, 0);
	}


	void SoundStream::setType(int type)
	{
		data->type = type;
	}


	int SoundStream::getType() const
	{
		return data->type;
	}


	bool SoundStream::hasEnded() const {
		// it hasn't loaded yet
		if(data->id >= 0)
		{
			if(data->playing)
				return false;
			else
				return true;
		}

		if (!data->finished) // stuff remain in ogg stream, definitely not finished
			return false;

		return (!data->playing);
	}


	const std::string &SoundStream::getFileName() const
	{
		return data->filename;
	}


	SoundLib::SoundLib()
	:	data(new Data(this))
	{
	}


	SoundLib::~SoundLib()
	{
	}


	SoundLib::Data::~Data()
	{
		bool wasInit = initialized;
		initialized = false;

		if (wasInit)
		{
			streamBuilder.streams.clear();
			channels.reset();

			ALCcontext *context = alcGetCurrentContext();
			ALCdevice *device = alcGetContextsDevice(context);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(context);
			alcCloseDevice(device);
		}
	}


	bool SoundLib::initialize()
	{
		assert(data != NULL);
		assert(!data->initialized);

		ALCdevice *device = alcOpenDevice(NULL);		// select the "preferred device"
		if (device)
		{
			ALCcontext *context = alcCreateContext(device, NULL);
			alcMakeContextCurrent(context);
		}
		else
		{
			LOG_ERROR("OpenAL device creation failed.");
		}

		alErrors();

		// first 3 used for default pos and velocity
		// last 3 for orient
		ALfloat temp[] =
		{ 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};

		alListenerfv(AL_POSITION,    temp);
		alErrors();

		alListenerfv(AL_VELOCITY,    temp);
		alErrors();

		temp[3] = -1.0;
		alListenerfv(AL_ORIENTATION, temp);
		alErrors();


		data->numchannels = std::max(data->maxHardwareChannels, data->softwareChannels);
		data->channels.reset(new Data::Channel[data->numchannels]);

		data->initialized = true;

		return true;
	}


	void SoundLib::setProperties(int mixrate_, int softwareChannels_)
	{
		data->mixrate = mixrate_;
		data->softwareChannels = softwareChannels_;
	}


	void SoundLib::setAcceleration(bool useHW, bool useEax_, int minHardwareChannels_, int maxHardwareChannels_)
	{
		data->useHardware = useHW;
		data->useEax = useEax_;
		data->minHardwareChannels = minHardwareChannels_;
		data->maxHardwareChannels = maxHardwareChannels_;
	}


	void SoundLib::setSpeakers(SpeakerType speakerType_)
	{
		data->speakerType = speakerType_;
	}


	void SoundLib::setSoundAPI(const char *api) {}

	void SoundLib::setGlobalVolume(float volume) {}


	void SoundLib::setFrequencyFactor(float scalar)
	{
		data->frequencyFactor = scalar;
	}


	void SoundLib::setListener(const VC3 &position, const VC3 &velocity, const VC3 &forwardDirection, const VC3 &upDirection)
	{
		ALfloat listenerOrigin[] =
		{ forwardDirection.x
			, forwardDirection.y
			, forwardDirection.z
			, upDirection.x
			, upDirection.y
			, upDirection.z
		};

		alListenerfv(AL_POSITION, &(position.x));
		alErrors();

		alListenerfv(AL_VELOCITY,  &(velocity.x));
		alErrors();

		alListenerfv(AL_ORIENTATION, listenerOrigin);
		alErrors();
	}


	void SoundLib::setSoundArea(const std::string &name) {}


	void SoundLib::update()
	{
		// update streams
		BOOST_FOREACH(SoundStream *stream, data->streams)
		{
			if (stream->data->playing   // it's in playing state
				&& stream->data->id < 0     // it has loaded
			   )
			{
				stream->data->update();
			}
		}

		// update storm streams
		data->streamBuilder.update();

		// clean up channels which have finished their current sample
		for (unsigned int i = 0; i < data->numchannels; i++)
		{
			Data::Channel &channel = data->channels[i];

			// is it even valid?
			if (channel.sound == NULL)
				continue;

			// has it finished?
			ALenum state = channel.source.getState();

			if (state == AL_STOPPED && channel.playing)
			{
				if (channel.looping)
				{
					alSourceRewind(channel.source);
					alErrors();

					channel.source.play();

					continue;
				}

				// clear it
				alSourcei(channel.source, AL_BUFFER, 0);
				channel.sound = NULL;
				channel.priority = -1;
				channel.playing = false;
			}

		}
	}


	SoundStream *SoundLib::createStream(const char *file)
	{
		if(!data->initialized)
			return NULL;


		int channelId = data->findFreeChannel(100);

		if (channelId < 0)
			return 0;

		Data::Channel &channel = data->channels[channelId];
		channel.loanedToStream = true;

		SoundStream *result = new SoundStream(new SoundStream::Data(channel.source, file));

		alErrors();

		alSource3f(result->data->source, AL_POSITION,        0.0, 0.0, 0.0);
		alErrors();

		alSource3f(result->data->source, AL_VELOCITY,        0.0, 0.0, 0.0);
		alErrors();

		alSource3f(result->data->source, AL_DIRECTION,       0.0, 0.0, 0.0);
		alErrors();

		alSourcef (result->data->source, AL_ROLLOFF_FACTOR,  0.0          );
		alErrors();

		alSourcei(result->data->source, AL_SOURCE_RELATIVE, AL_TRUE      );
		alErrors();

		alSourcei(result->data->source, AL_LOOPING, AL_FALSE);
		alErrors();


		result->data->soundLib = this;

		data->streams.push_back(result);

		return result;
	}


	Sound *SoundLib::loadSample(const char *file)
	{
		assert(this != NULL);
		assert(data);

		if(!data->initialized)
			return NULL;

		if (file == NULL)
		{
			LOG_WARNING("SoundLib::loadSample - Null filename parameter.");
			return NULL;
		}

		Sound *newSound = new Sound(new Sound::Data(file, 0));

		return newSound;
	}


	// create a sound in paused state
	int SoundLib::createSound(Sound *sound, int priority)
	{
		assert(this != NULL);
		assert(this->data != NULL);

		alErrors();

		if(!sound)
			return -1;

		if (!sound->data)
			return -1;

		if (sound->data->fileNotFound)
		{
			LOG_DEBUG("file not found");
			return -1;
		}

		int channelId = data->findFreeChannel(priority);

		if (channelId < 0)
		{
			LOG_DEBUG("no free channel");
			return -1;
		}

		Data::Channel &channel = data->channels[channelId];
		channel.priority = priority;
		channel.sound = sound;

		const ALfloat temp[] = {0.0, 0.0, 0.0};

		alSourcei(channel.source, AL_BUFFER, sound->data->sample);
		alErrors();

		alSourcef(channel.source, AL_PITCH, 1.0);
		alErrors();

		alSourcef(channel.source, AL_GAIN, 1.0);
		alErrors();

		alSourcefv(channel.source, AL_POSITION, temp);
		alErrors();

		alSourcefv(channel.source, AL_VELOCITY, temp);
		alErrors();

		alSourcei(channel.source, AL_LOOPING, AL_FALSE);
		alErrors();

		channel.playing = false;  // created in paused state
		channel.looping = false;  // clear the looping state in case someone left it on

		return channelId;
	}


	// set the sound to playing state
	void SoundLib::playSound(int sound)
	{
		alErrors();

		if (sound < 0 || sound >= data->numchannels || data->channels[sound].sound == NULL)
		{
			LOG_DEBUG("Play failed");
			return;
		}

		if (!data->channels[sound].playing)
		{
			// not playing, call play
			data->channels[sound].playing = true;
			data->channels[sound].source.play();
		} else {
		}
	}


	void SoundLib::setSoundLoop(int sound, bool loop)
	{
		alErrors();

		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return;
		}

		if (data->channels[sound].sound == NULL)
			return;

		data->channels[sound].looping = loop;
	}


	void SoundLib::setSoundPaused(int sound, bool pause)
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return;
		}

		if (data->channels[sound].sound == NULL)
			return;

		data->channels[sound].playing = false;

		data->channels[sound].source.pause();

	}


	void SoundLib::setSoundVolume(int sound, float value)
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return;
		}

		if (data->channels[sound].sound == NULL)
			return;

        alSourcef(data->channels[sound].source, AL_GAIN, value);
		alErrors();

	}


	// cannot be changed in openal
	void SoundLib::setSoundFrequency(int sound, int value) {}


	void SoundLib::setSound3D(int sound, const VC3 &position, const VC3 &velocity)
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return;
		}

		if (data->channels[sound].sound == NULL)
			return;

		alSourcefv(data->channels[sound].source, AL_POSITION, &(position.x));
		alErrors();

		alSourcefv(data->channels[sound].source, AL_VELOCITY, &(velocity.x));
		alErrors();
	}


	void SoundLib::stopSound(int sound)
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return;
		}

		if (data->channels[sound].sound == NULL)
			return;

		data->channels[sound].source.stop();
		alSourcei(data->channels[sound].source, AL_BUFFER, 0);

		data->channels[sound].sound = NULL;
		data->channels[sound].playing = false;
		data->channels[sound].looping = false;
	}


	float SoundLib::getSoundVolume(int sound) const
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return 0;
		}

		if (data->channels[sound].sound == NULL)
			return 0.0;

		float vol = 0.0;
		alGetSourcef(data->channels[sound].source, AL_GAIN, &vol);
		alErrors();

		return vol;
	}


	int SoundLib::getSoundFrequency(int sound) const
	{
		if(sound < 0)
		{
			assert(!"Invalid sound handle");
			return 0;
		}

		if (data->channels[sound].sound == NULL)
			return 0;

		return data->channels[sound].sound->data->freq;
	}


	int SoundLib::getSoundTime(int sound) const
	{
		if(sound < 0)
		{
			//assert(!"Invalid sound handle");
			return 0;
		}

		if (data->channels[sound].sound == NULL)
			return 0;

		float time = 0.0;
		alGetSourcef(data->channels[sound].source, AL_SEC_OFFSET, &time);
		alErrors();

		return (int)(time * 1000);
	}


	bool SoundLib::isSoundPlaying(int sound) const
	{
		if(sound < 0)
		{
			//assert(!"Invalid sound handle");
			return false;
		}

		if (data->channels[sound].sound == NULL)
			return false;

		if (data->channels[sound].playing)
		{
			if (data->channels[sound].looping)
			{
				// if it's looping, then it's definitely playing
				return true;
			}

			ALenum state = data->channels[sound].source.getState();

			if (state == AL_PLAYING)
				return true;
		}

		return false;
	}


boost::shared_ptr<IStorm3D_Stream> SoundLib::createStormStream(bool stereo_, int frequency_, int bits_, float volume)
{
	int channelId = data->findFreeChannel(100);

	if (channelId < 0)
		return boost::shared_ptr<IStorm3D_Stream>();

	Data::Channel &channel = data->channels[channelId];
	channel.loanedToStream = true;

	boost::shared_ptr<StormStream> retval(new StormStream(stereo_, frequency_, bits_, volume, channel.source, this));
	data->streamBuilder.streams.push_back(retval);
	return boost::static_pointer_cast<IStorm3D_Stream>(retval);
}


}

