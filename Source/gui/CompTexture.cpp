#include "CompTexture.h"

namespace gui
{
	CompTexture::CompTexture(Utils& u, const void* _data, int _size, float _gain, int _scale) :
		Comp(u),
		data(_data),
		size(_size),
		scale(_scale),
		gain(_gain),
		img()
	{
		setInterceptsMouseClicks(false, false);
		setBufferedToImage(true);
	}

	void CompTexture::paint(Graphics& g)
	{
		for (auto y = 0; y < getHeight(); y += img.getHeight())
			for (auto x = 0; x < getWidth(); x += img.getWidth())
				g.drawImageAt(img, x, y);
	}

	void CompTexture::resized()
	{
		const auto tile = ImageCache::getFromMemory(data, size);
		img = Image(Image::ARGB, tile.getWidth() * scale, tile.getHeight() * scale, false);
		fixStupidJUCEImageThingie(img);
		for (auto y = 0; y < img.getHeight(); ++y)
			for (auto x = 0; x < img.getWidth(); ++x)
			{
				const auto pxl = tile.getPixelAt(x, y);
				const auto b = pxl.getBrightness() * gain;
				for (auto yy = 0; yy < scale; ++yy)
				{
					const auto yyy = y * scale + yy;
					for (auto xx = 0; xx < scale; ++xx)
					{
						const auto xxx = x * scale + xx;
						img.setPixelAt(xxx, yyy, getColour(CID::Darken).withAlpha(b));
					}
				}
			}
	}
}