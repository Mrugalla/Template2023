#include "BgImage.h"

namespace gui
{
    namespace fx
    {
        void vignette(Image& img, Colour col, int w, int h, float midPoint = .9f)
        {
            for (auto y = 0; y < h; ++y)
            {
                const auto yF = static_cast<float>(y);
                const auto yNorm = yF / static_cast<float>(h);
                const auto yOff = yNorm - .5f;
                const auto ySqr = yOff * yOff;
                const auto y2 = 4.f * ySqr;

                for (auto x = 0; x < w; ++x)
                {
                    const auto xF = static_cast<float>(x);
                    const auto xNorm = xF / static_cast<float>(w);
                    const auto xOff = xNorm - .5f;
                    const auto xSqr = xOff * xOff;
                    const auto x2 = 4.f * xSqr;

                    auto pxl = img.getPixelAt(x, y);

                    auto mag = x2 * y2;
                    mag = std::pow(mag, 1.f - midPoint);

                    pxl = pxl.interpolatedWith(col, mag);

                    img.setPixelAt(x, y, pxl);
                }
            }
        }
    }

    namespace create
    {
        void pulsar(Image& img, float thicc, int w, int h)
		{
            Random rand;

            img = Image(Image::ARGB, w, h, true);
			Graphics g(img);
			g.setColour(juce::Colours::turquoise);
			
            const auto wF = static_cast<float>(w);
            const auto hF = static_cast<float>(h);
            const auto numLines = rand.nextInt() & 63 + 32;
            const auto numLinesF = static_cast<float>(numLines);
            const auto h333 = hF / 3.f;

            Path path;

            auto iNormRand = rand.nextFloat();
            auto iSined3Rand = rand.nextFloat() * 2.8f;

            for (auto i = 0; i < numLines; ++i)
            {
                const auto iF = static_cast<float>(i);
                const auto iNorm = iF / numLinesF * iNormRand;
                const auto iSined = .5f * std::cos(iNorm * Pi + Pi) + .5f;

                const auto xStart = 0.f;
                const auto yStart = h333 + iSined * h333;
                const auto xEnd = wF;
                const auto yEnd = hF - iSined * h333;

                const auto iSined3 = .5f * std::cos(iSined3Rand * iNorm * Pi + Pi) + .5f;
                const auto cA = h333 + iSined3 * h333;
                const auto cB = hF - iSined3 * h333;

                path.startNewSubPath(xStart, yStart);
                path.quadraticTo(cA, cB, xEnd, yEnd);
                
            }

            Stroke stroke(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::butt);
            g.strokePath(path, stroke);
            
            fx::vignette(img, juce::Colours::black, w, h);
		}
    }

    void loadImage(const File& file, Image& img)
    {
        const auto nFile = file.getParentDirectory();
        const auto findFiles = File::TypesOfFileToFind::findFiles;
        const auto wildCard = "*.png";
        for (const auto& f : nFile.findChildFiles(findFiles, true, wildCard))
        {
            if (f.getFileName() == "bgImage.png")
            {
                auto nImg = ImageFileFormat::loadFrom(f);
                if (nImg.isValid())
                {
                    img = nImg;
                    return;
                }
            }
        }
    }

    bool componentOk(Component& comp) noexcept
    {
        return comp.getWidth() > 0 && comp.getHeight() > 0;
    }

    void saveImage(const File& file, const Image& img)
    {
        const auto nFile = file.getParentDirectory().getChildFile("bgImage.png");
        if (nFile.exists())
            nFile.deleteFile();
        FileOutputStream stream(nFile);
        PNGImageFormat pngWriter;
        pngWriter.writeImageToStream(img, stream);
    }

    BgImage::BgImage(Utils& u) :
        Comp(u),
        img(),
        refreshButton(utils, "Refresh", "Click here to request a new background image.")
    {
        setInterceptsMouseClicks(false, true);

        makeTextButton(refreshButton, juce::Colours::limegreen);
        refreshButton.onClick = [&](const Mouse&)
        {
            updateBgImage(false);
            repaint();
        };

        updateBgImage(false);
        setOpaque(true);

        addCallback(Callback([&]()
        {
            if (!img.isValid())
                return;
            const auto w = getWidth();
            const auto h = getHeight();
            if (img.getWidth() == w && img.getHeight() == h)
                return;

            updateBgImage(false);
            repaint();
        }, kUpdateBoundsCB), cbFPS::k3_75);
    }

    void BgImage::paint(Graphics& g)
    {
        g.fillAll(juce::Colours::black);
        g.drawImageAt(img, 0, 0, false);
    }

    void BgImage::resized()
    {
        layout.resized(getLocalBounds());

        if (!img.isValid())
            updateBgImage(true);
    }

    void BgImage::updateBgImage(bool forcedLoad)
    {
        auto& props = utils.audioProcessor.state.props;
        auto& user = *props.getUserSettings();
        const auto& file = user.getFile();
        if (!forcedLoad)
            loadImage(file, img);

        if (componentOk(*this))
        {
            create::pulsar(img, utils.thicc, getWidth(), getHeight());
            saveImage(file, img);
        }
    }
}

/*
* 
todo:

fx::vignette is very edge-orientated
create::pulsar doesn't look like a pulsar but like a generic lines bg and has no randomization yet

*/