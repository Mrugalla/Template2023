#pragma once
#include "../Using.h"

namespace dsp
{
    struct Sysex
    {
		using ByteArray = std::array<uint8_t, 128>;

        static ByteArray makeBytesOnset() noexcept
        {
            return { 0x00, 0x69, 0x42, 0x01 };
        }

        struct Info
        {
            uint8_t* data;
            int size;
        };

        Sysex() :
            bytes(),
            length(0)
        {
            for(auto& d : bytes)
				d = 0;
        }

        Info encode(int val) noexcept
        {
            auto i = 0;
            while (val > 0 && i < bytes.size() - 1)
            {
                bytes[i] = val & 0x7F;
                val >>= 7;
                ++i;
            }
            bytes[i] = 0;
            length = i;
            return Info( bytes.data(), getSize() );
        }

        MidiMessage midify(int val) noexcept
        {
            const auto info = encode(val);
            return MidiMessage::createSysExMessage(info.data, info.size);
        }

        MidiMessage midify(const ByteArray& b) noexcept
        {
            length = static_cast<int>(b.size());
            for (auto i = 0; i < length; ++i)
                bytes[i] = b[i];
            return MidiMessage::createSysExMessage(bytes.data(), getSize());
        }

        static int decode(const uint8_t* _bytes, int size) noexcept
        {
            int val = 0;
            for (auto i = size - 1; i >= 0; --i)
            {
                val <<= 7;
                val |= _bytes[i] & 0x7F;
            }
			return val;
		}

        static int decode(const ByteArray& b) noexcept
        {
			return decode(b.data(), static_cast<int>(b.size()));
        }

    private:
        ByteArray bytes;
        int length;

		int getSize() const noexcept
        {
            static constexpr auto Byte = sizeof(uint8_t);
            return length * sizeof(Byte);
        }
    };
}
// todo:
// SYSEX ID TABLE https://midi.org/SysExIDtable