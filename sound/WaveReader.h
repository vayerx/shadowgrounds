// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef INCLUDED_WAVEREADER_H
#define INCLUDED_WAVEREADER_H

#include <boost/scoped_ptr.hpp>
#include <string>

namespace sfx {
    class AmplitudeArray;

    class WaveReader {
        struct Data;
        boost::scoped_ptr<Data> data;

    public:
        WaveReader(const std::string &file);
        ~WaveReader();

        void readAmplitudeArray(int tickTime, AmplitudeArray &array);

        // fill specified vector with sound data
        void decodeAll(std::vector<char> &target);

        // return sampling frequency
        unsigned int getFreq() const;

        // return number of channels
        unsigned int getChannels() const;

        // return number of bits per sample (8 or 16)
        unsigned int getBits() const;

        // returns true if wave file succesfully loaded
        operator bool() const;
    };

} // sfx

#endif
