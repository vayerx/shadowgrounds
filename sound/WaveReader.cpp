
#include "precompiled.h"

// Copyright 2002-2004 Frozenbyte Ltd.

#include "WaveReader.h"
#include "AmplitudeArray.h"
#include "../system/Logger.h"
#include "../util/assert.h"
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include "../filesystem/file_package_manager.h"
#include "../filesystem/input_stream.h"
#include <windows.h>

using namespace boost;
using namespace std;
using namespace frozenbyte;

namespace sfx {
namespace {

	void logParseError(const std::string &file, const std::string &info)
	{
#ifdef _MSC_VER
#pragma message(" ** Wave reader errors disabled as they seem to be always triggered ** ")
#endif

		/*
		string errorString = "Error parsing wave file: ";
		errorString += file;

		Logger::getInstance()->error(errorString.c_str());
		if(!info.empty())
			Logger::getInstance()->info(info.c_str());
		*/
	}

	struct CharDeleter
	{
		void operator () (WAVEFORMATEX *ptr)
		{
			if(!ptr)
				return;

			CHAR *charPtr = reinterpret_cast<CHAR *> (ptr);
			delete[] charPtr;
		}
	};

} // unnamed


struct WaveReader::Data
{
	string fileName;

	shared_ptr<WAVEFORMATEX> waveFormat;
	HMMIO ioHandle;
	MMCKINFO riffChunk;

	boost::shared_array<char> buffer;

	Data()
	:	ioHandle(0)
	{
	}

	~Data()
	{
		free();
	}

	void open(const string &file)
	{
		fileName = file;

		filesystem::InputStream fileStream = filesystem::FilePackageManager::getInstance().getFile(file);
		if(!fileStream.isEof())
		{
			int size = fileStream.getSize();
			buffer.reset(new char[size]);
			fileStream.read(buffer.get(), size);

			MMIOINFO info = { 0 };
			info.pchBuffer = buffer.get();
			info.fccIOProc = FOURCC_MEM;
			info.cchBuffer = size;

			ioHandle = mmioOpen(0, &info, MMIO_READ);
		}

		if(!ioHandle)
		{
			string errorString = "Cannot open wave file: ";
			errorString += file;

			Logger::getInstance()->warning(errorString.c_str());
		}

		/*
		ioHandle = mmioOpen(const_cast<char *> (fileName.c_str()), NULL, MMIO_ALLOCBUF | MMIO_READ );
		if(!ioHandle)
		{
			string errorString = "Cannot open wave file: ";
			errorString += file;

			Logger::getInstance()->warning(errorString.c_str());
		}
		*/
	}

	void findPosition()
	{
		MMCKINFO chunkInfo = { 0 };
		PCMWAVEFORMAT pcmWaveFormat = { 0 };

		if(mmioDescend(ioHandle, &riffChunk, NULL, 0) != MMSYSERR_NOERROR)
		{
			logParseError(fileName, "findPosition::mmioDescend");
			free();

			return;
		}

		if((riffChunk.ckid != FOURCC_RIFF) || (riffChunk.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
		{
			logParseError(fileName, "findPosition::waveChunkCheck");
			free();

			return;
		}

		chunkInfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		if(mmioDescend(ioHandle, &chunkInfo, &riffChunk, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
		{
			logParseError(fileName, "findPosition::fmtChunkCheck");
			free();

			return;
		}

		if(chunkInfo.cksize < sizeof(PCMWAVEFORMAT))
		{
			logParseError(fileName, "findPosition::chunkInfoSize");
			free();

			return;
		}

		if(mmioRead(ioHandle, (HPSTR) &pcmWaveFormat, sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat))
		{
			logParseError(fileName, "findPosition::waveFormatSize");
			free();

			return;
		}

		if(pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
		{
			waveFormat.reset(reinterpret_cast<WAVEFORMATEX *> (new CHAR[sizeof(WAVEFORMATEX)]), CharDeleter());
			memcpy(waveFormat.get(), &pcmWaveFormat, sizeof(pcmWaveFormat));
			waveFormat->cbSize = 0;
		}
		else
		{
			WORD extraBytes = 0L;
			if(mmioRead(ioHandle, (CHAR*) &extraBytes, sizeof(WORD)) != sizeof(WORD))
			{
				logParseError(fileName, "findPosition::readExtreByteAmount");
				free();

				return;
			}

			waveFormat.reset(reinterpret_cast<WAVEFORMATEX *> (new CHAR[sizeof(WAVEFORMATEX) + extraBytes]), CharDeleter());
			memcpy(waveFormat.get(), &pcmWaveFormat, sizeof(pcmWaveFormat));
			waveFormat->cbSize = extraBytes;

			if(mmioRead(ioHandle, (CHAR*)(((BYTE*)&(waveFormat->cbSize))+sizeof(WORD)), extraBytes ) != extraBytes)
			{
				logParseError(fileName, "findPosition::readExtreBytes");
				free();

				return;
			}
		}

		if(mmioAscend(ioHandle, &chunkInfo, 0) != MMSYSERR_NOERROR)
		{
			logParseError(fileName, "findPosition::mmioAscend");
			free();

			return;
		}
	}

	void resetRead()
	{
		MMCKINFO chunkInfo = { 0 };
		if(-1 == mmioSeek(ioHandle, riffChunk.dwDataOffset + sizeof(FOURCC), SEEK_SET))
			return;

		// Search the input file for the 'data' chunk.
		chunkInfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if(0 != mmioDescend(ioHandle, &chunkInfo, &riffChunk, MMIO_FINDCHUNK))
			return;
	}

	void readAmplitudeArray(int tickTime, AmplitudeArray &array)
	{
		FB_ASSERT(tickTime >= 10);
		if(!ioHandle)
			return;

		if(waveFormat->wBitsPerSample != 8 && waveFormat->wBitsPerSample != 16)
		{
			logParseError(fileName, "readAmplitudeArray::unsupported bits per sample amount");
			free();

			return;
		}

		if(waveFormat->nChannels != 1 && waveFormat->nChannels != 2)
		{
			logParseError(fileName, "readAmplitudeArray::unsupported channel amount");
			free();

			return;
		}

		int sampleSize = waveFormat->nSamplesPerSec * waveFormat->nChannels * waveFormat->wBitsPerSample / 8;
		int amplitudeSamplesPerSecond = 1000 / tickTime;
		int amplitudeSampleDelta = sampleSize / amplitudeSamplesPerSecond;
		int amplitudeSampleAmount = riffChunk.cksize / amplitudeSampleDelta + 1;

        MMIOINFO statusInfo = { 0 };
        if(mmioGetInfo(ioHandle, &statusInfo, 0) != MMSYSERR_NOERROR)
		{
			logParseError(fileName, "readAmplitudeArray::mmioGetInfo");
			free();

			return;
		}

		array.setSampleAmount(amplitudeSampleAmount);
		int index = 0;

		unsigned int bytesRead = 0;
		int loopIndex = 0;

		//vector<unsigned char> sampleBuffer(1000);
		vector<unsigned char> sampleBuffer(amplitudeSampleDelta / 2);
		int sampleIndex = 0;

		while(bytesRead < riffChunk.cksize)
		{
			++loopIndex;
            if(statusInfo.pchNext == statusInfo.pchEndRead)
            {
                if(mmioAdvance(ioHandle, &statusInfo, MMIO_READ) != MMSYSERR_NOERROR)
				{
					logParseError(fileName, "readAmplitudeArray::mmioAdvance");
					free();

					return;
				}

                if(statusInfo.pchNext == statusInfo.pchEndRead)
				{
					//logParseError(fileName, "readAmplitudeArray::unexpected end");
					free();

					return;
				}
            }

			int amplitude = 0;
			//for(int i = 0; i < waveFormat->nChannels; ++i)
			{
				if(waveFormat->wBitsPerSample == 8)
				{
					amplitude += *((BYTE *) statusInfo.pchNext);
					assert(statusInfo.pchNext != statusInfo.pchEndRead);
					++statusInfo.pchNext;

					bytesRead += 1;
				}
				else if(waveFormat->wBitsPerSample == 16)
				{
					signed short int a = *((unsigned short *) statusInfo.pchNext);
					if(a < 0)
						a = -a;
					unsigned char ash = a >> 7;
					assert(statusInfo.pchNext != statusInfo.pchEndRead);
					++statusInfo.pchNext;
					assert(statusInfo.pchNext != statusInfo.pchEndRead);
					++statusInfo.pchNext;

					//*((BYTE *) &a) = *((BYTE *) statusInfo.pchNext);
					//++statusInfo.pchNext;
					//*(((BYTE *) &a) + 1) = *((BYTE *) statusInfo.pchNext);
					//++statusInfo.pchNext;

					bytesRead += 2;
					amplitude += ash;
				}
				else
					FB_ASSERT(!"Shouldnt get here.");
			}

			//if(waveFormat->nChannels == 2)
			//	amplitude >>= 1;

			if(waveFormat->nSamplesPerSec < 44100 || loopIndex % 2 == 0)
			{
				//if(amplitude > arrayAmplitude)
				//	arrayAmplitude = amplitude;

				sampleBuffer[sampleIndex] = amplitude;
				if(++sampleIndex >= int(sampleBuffer.size()))
					sampleIndex = 0;
			}

			// Do stuff
			if(bytesRead % amplitudeSampleDelta == 0)
			{
				int avg = 0;
				int max = 0;
				for(unsigned int i = 0; i < sampleBuffer.size(); ++i)
				{
					unsigned char v = sampleBuffer[i];
					avg += v;
					if(v > max)
						max = v;
				}

				avg /= sampleBuffer.size();
				array.setSampleAmplitude(index++, (avg / 2) + (max / 2));
				//array.setSampleAmplitude(index++, (2 * avg / 3) + (1 * max / 3));
				//array.setSampleAmplitude(index++, avg);
				//array.setSampleAmplitude(index++, max);
			}
		}

        if(mmioSetInfo(ioHandle, &statusInfo, 0) != MMSYSERR_NOERROR)
		{
			logParseError(fileName, "readAmplitudeArray::mmioSetInfo");
			free();

			return;
		}

		array.update();
	}

	void free()
	{
		if(ioHandle)
		{
			mmioClose(ioHandle, 0);
			ioHandle = 0;
		}
	}
};

WaveReader::WaveReader(const string &file)
{
	scoped_ptr<Data> newData(new Data());
	data.swap(newData);

	data->open(file);
	data->findPosition();
	data->resetRead();
}

WaveReader::~WaveReader()
{
}

void WaveReader::readAmplitudeArray(int tickTime, AmplitudeArray &array)
{
	data->readAmplitudeArray(tickTime, array);
}

} // sfx
