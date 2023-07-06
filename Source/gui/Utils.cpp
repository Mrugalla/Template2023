#include "Utils.h"

namespace gui
{
	Utils::Utils(Component& _pluginTop, Processor& _audioProcessor) :
		eventSystem(),
		pluginTop(_pluginTop),
		audioProcessor(_audioProcessor),
		params(audioProcessor.params),
		thicc(2.f)
	{
	}

	std::vector<Param*>& Utils::getAllParams() noexcept
	{
		return params.data();
	}

	const std::vector<Param*>& Utils::getAllParams() const noexcept
	{
		return params.data();
	}

	ValueTree Utils::getState() const noexcept
	{
		return audioProcessor.state.state;
	}

	//void Utils::assignMIDILearn(PID pID) noexcept
	//{
	//	audioProcessor.midiManager.midiLearn.assignParam(params[pID]);
	//}

	//void Utils::removeMIDILearn(PID pID) noexcept
	//{
	//	audioProcessor.midiManager.midiLearn.removeParam(params[pID]);
	//}

	//const audio::MIDILearn& Utils::getMIDILearn() const noexcept
	//{
	//	return audioProcessor.midiManager.midiLearn;
	//}

	void Utils::resized()
	{
		auto a = std::min(pluginTop.getWidth(), pluginTop.getHeight());
		auto t = static_cast<float>(a) * .004f;
		thicc = t < 1.f ? 1.f : t;
	}

	float Utils::getDragSpeed() const noexcept
	{
		const auto height = static_cast<float>(pluginTop.getHeight());
		const auto speed = DragSpeed * height;
		return speed;
	}

	float Utils::fontHeight() const noexcept
	{
		const auto w = static_cast<float>(pluginTop.getWidth());
		const auto h = static_cast<float>(pluginTop.getHeight());

		const auto avr = (w + h) * .5f;
		const auto norm = (avr - 500.f) / 500.f;
		return std::floor(8.5f + norm * 5.f);
	}

	void Utils::giveDAWKeyboardFocus()
	{
		pluginTop.giveAwayKeyboardFocus();
	}

	ValueTree Utils::savePatch()
	{
		audioProcessor.pluginProcessor.savePatch();
		return audioProcessor.state.state;
	}

	void Utils::loadPatch(const ValueTree& vt)
	{
		audioProcessor.state.loadPatch(vt);
		audioProcessor.pluginProcessor.loadPatch();
	}

	Props& Utils::getProps() noexcept
	{
		return *audioProcessor.state.props.getUserSettings();
	}

	Point Utils::getScreenPosition() const noexcept
	{
		return pluginTop.getScreenPosition();
	}

	void hideCursor()
	{
		auto mms = juce::Desktop::getInstance().getMainMouseSource();
		mms.enableUnboundedMouseMovement(true, false);
	}

	void showCursor(const Component& comp)
	{
		auto mms = juce::Desktop::getInstance().getMainMouseSource();
		centreCursor(comp, mms);
		mms.enableUnboundedMouseMovement(false, true);
	}

	void centreCursor(const Component& comp, juce::MouseInputSource& mms)
	{
		const Point centre(comp.getWidth() / 2, comp.getHeight() / 2);
		mms.setScreenPosition((comp.getScreenPosition() + centre).toFloat());
	}

	void appendRandomString(String& str, Random& rand, int length, const String& legalChars)
	{
		const auto max = static_cast<float>(legalChars.length() - 1);

		for (auto i = 0; i < length; ++i)
		{
			auto idx = static_cast<int>(rand.nextFloat() * max);
			str += legalChars[idx];
		}
	}
}