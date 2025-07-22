#include "Using.h"

namespace gui
{
	bool isLineBreak(WChar wChar) noexcept
	{
		return wChar == '\n' || wChar == '\r' || wChar == '\r\n';
	}

	bool isTextCharacter(WChar wchar) noexcept
	{
		return wchar >= 0x20 && wchar <= 0x7E;
	}

	namespace font
	{
		// backend:

		Font getFont(const char* ttf, size_t size)
		{
			const auto typeface = juce::Typeface::createSystemTypefaceFor(ttf, size);
			const auto fontOptions = FontOptions(typeface);
			return Font(fontOptions);
		}

		Font wdxl()
		{
			return getFont(BinaryData::WDXLLubrifontTCRegular_ttf, BinaryData::WDXLLubrifontTCRegular_ttfSize);
		}

		Font bigfast()
		{
			auto f = getFont(BinaryData::BigFastDemo3l6np_ttf, BinaryData::BigFastDemo3l6np_ttfSize);
			f.setDescentOverride(0.f);
			return f;
		}

		// front end:

		Font nel()
		{
			return getFont(BinaryData::nel19_ttf, BinaryData::nel19_ttfSize);
		}

		Font flx()
		{
			return getFont(BinaryData::felixhand_02_ttf, BinaryData::felixhand_02_ttfSize);
		}

		Font text()
		{
			return wdxl();
		}

		Font headline()
		{
			return bigfast();
		}
	}
}