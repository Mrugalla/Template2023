#include "LabelPluginRecorder.h"

namespace gui
{
	Image LabelPluginRecorder::ImgData::getImage()
	{
		return ImageFileFormat::loadFrom(data, size).createCopy();
	}

	LabelPluginRecorder::LabelPluginRecorder(Utils& u, Recorder& _recorder) :
		Label(u, "pluginrecorder"),
		bgImg(),
		recorder(_recorder),
		dragImage(Image::ARGB, 20, 20, true),
		scaledImage(dragImage),
		file(),
		imgData(nullptr, 0)
	{
		setTooltip("I wonder what happens when you drag this into your DAW! ;)");

		const auto fps = cbFPS::k15;
		add(Callback([&, inc = msToInc(AniLengthMs, fps)]()
		{
			auto phase = callbacks[kHoverAni].phase;
			const auto hovering = isMouseOverOrDragging();
			if (hovering)
			{
				phase += inc;
				if (phase > 1.f)
				{
					phase = 1.f;
					callbacks[kHoverAni].active = false;
				}
			}
			else
			{
				phase -= inc;
				if (phase < 0.f)
				{
					phase = 0.f;
					callbacks[kHoverAni].active = false;
				}
			}
			callbacks[kHoverAni].phase = phase;
			repaint();
		}, kHoverAni, fps, false));
		callbacks[kHoverAni].phase = 0.f;

		addEvt([&](evt::Type eType, const void*)
		{
			if (eType == evt::Type::ThemeUpdated)
				updateImage();
		});

		Path path;
		path.startNewSubPath(10, 0);
		path.quadraticTo(10, 10, 20, 10);
		path.quadraticTo(10, 10, 10, 20);
		path.quadraticTo(10, 10, 0, 10);
		path.quadraticTo(10, 10, 10, 0);
		const Stroke stroke(2.f, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);
		Graphics g{ dragImage };
		setCol(g, CID::Hover);
		g.strokePath(path, stroke);
	}

	void LabelPluginRecorder::paint(Graphics& g)
	{
		if (bgImg.isValid())
		{
			g.fillAll(getColour(CID::Bg));
			const auto thicc = utils.thicc;
			const auto thicc2 = thicc * 2.f;
			const auto hoverAni = callbacks[kHoverAni].phase;
			const auto reduced = thicc2 + hoverAni * (thicc - thicc2);
			g.drawImage(bgImg, getLocalBounds().toFloat().reduced(reduced), RectanglePlacement::stretchToFit, false);
			return;
		}
		Label::paint(g);
	}

	void LabelPluginRecorder::mouseEnter(const Mouse& mouse)
	{
		Label::mouseEnter(mouse);
		callbacks[kHoverAni].start(callbacks[kHoverAni].phase);
	}

	void LabelPluginRecorder::mouseExit(const Mouse& mouse)
	{
		Label::mouseExit(mouse);
		callbacks[kHoverAni].start(callbacks[kHoverAni].phase);
	}

	void LabelPluginRecorder::mouseDown(const Mouse& mouse)
	{
		saveWav();
		const Var sourceDescription("pluginrecorder");
		Point imageOffsetFromMouse(0, 0);
		if (!file.existsAsFile())
			return;
		startDragging
		(
			sourceDescription,
			this,
			scaledImage,
			true,
			&imageOffsetFromMouse,
			&mouse.source
		);
	}

	bool LabelPluginRecorder::shouldDropFilesWhenDraggedExternally(const DnDSrc&,
		StringArray& files, bool& canMoveFiles)
	{
		files.clearQuick();
		files.add(file.getFullPathName());
		canMoveFiles = false;
		return true;
	}

	void LabelPluginRecorder::getTheFile()
	{
		const auto& user = *utils.audioProcessor.state.props.getUserSettings();
		const auto settingsFile = user.getFile();
		const auto userDirectory = settingsFile.getParentDirectory();
		file = userDirectory.getChildFile("muggie bounce.wav");
	}

	bool LabelPluginRecorder::saveWav(const dsp::AudioBuffer& buffer)
	{
		getTheFile();
		if (file.existsAsFile())
			file.deleteFile();
		file.create();
		WavAudioFormat format;
		const auto numChannels = utils.audioProcessor.getBus(true, 0)->getNumberOfChannels();
		const auto Fs = utils.audioProcessor.getSampleRate();
		juce::AudioFormatWriterOptions options;
		options = options.withBitsPerSample(24)
			.withSampleRate(Fs)
			.withNumChannels(numChannels);
		std::unique_ptr<juce::OutputStream> outStream(new FileOutputStream(file));
		const auto writer = format.createWriterFor(outStream, options);
		const auto numSamples = buffer.getNumSamples();
		if (!writer)
			return false;
		writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
		return true;
	}

	void LabelPluginRecorder::saveWav()
	{
		utils.audioProcessor.suspendProcessing(true);
		const auto& recording = recorder.getRecording();
		utils.audioProcessor.suspendProcessing(false);
		const auto success = saveWav(recording);
		if (success)
			return getTheFile();
		file = File();
	}

	void LabelPluginRecorder::setImage(const void* data, int size)
	{
		imgData = { data, size };
		updateImage();
	}

	void LabelPluginRecorder::updateImage()
	{
		auto nImg = imgData.getImage();
		if (!nImg.isValid())
			return;
		fixStupidJUCEImageThingie(nImg);
		const auto bgCol = getColour(CID::Bg);
		const auto lineCol = getColour(CID::Interact);
		for (auto y = 0; y < nImg.getHeight(); ++y)
		{
			for (auto x = 0; x < nImg.getWidth(); ++x)
			{
				const auto pxl = nImg.getPixelAt(x, y);
				const auto white = math::tanhApprox(2.f * pxl.getBrightness());
				const auto nPxl = bgCol.interpolatedWith(lineCol, white);
				nImg.setPixelAt(x, y, nPxl);
			}
		}
		bgImg = nImg;
		repaint();
	}
}