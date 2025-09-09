#pragma once
#include "../Using.h"

namespace dsp
{
    struct Sysex
    {
		using ByteArray = std::array<uint8_t, 128>;

        struct Info
        {
            uint8_t* data;
            int size;
        };

        Sysex() :
            bytes(),
            length(3)
        {
            // dev id i guess:
            bytes[0] = 0x29;
            bytes[1] = 0x10;
			bytes[2] = 0x91 & 127;
        }
          
        Sysex(const uint8_t* data, const int size) :
            bytes(),
            length(size)
        {
            for (auto i = 0; i < length; ++i)
                bytes[i] = data[i];
        }

        MidiMessage midify() noexcept
        {
            return MidiMessage::createSysExMessage(bytes.data(), length);
        }

        void makeBytesOnset() noexcept
        {
			bytes[3] = 0x01;
            length = 4;
        }

        bool operator==(const Sysex& o) const noexcept
        {
            if (length != o.length)
                return false;
            for (auto i = 0; i < length; ++i)
                if (bytes[i] != o.bytes[i])
                    return false;
            return true;
		}
    private:
        ByteArray bytes;
        int length;
    };
}
// SYSEX ID TABLE https://midi.org/SysExIDtable