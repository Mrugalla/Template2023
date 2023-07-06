#pragma once
#include "Using.h"
#include "Events.h"
//#include "../audio/MIDILearn.h"

namespace gui
{
	class Utils
	{
		static constexpr float DragSpeed = .5f;
	public:
		/* pluginTop, processor */
		Utils(Component&, Processor&);

		std::vector<Param*>& getAllParams() noexcept;
		const std::vector<Param*>& getAllParams() const noexcept;

		juce::ValueTree getState() const noexcept;

		//void assignMIDILearn(PID pID) noexcept;
		//void removeMIDILearn(PID pID) noexcept;
		//const audio::MIDILearn& getMIDILearn() const noexcept;

		float getDragSpeed() const noexcept;

		float fontHeight() const noexcept;

		Point getScreenPosition() const noexcept;

		void resized();

		ValueTree savePatch();

		void loadPatch(const ValueTree&);

		Props& getProps() noexcept;

		void giveDAWKeyboardFocus();

		evt::System eventSystem;
		Component& pluginTop;
		Processor& audioProcessor;
		Params& params;
		float thicc;
	};

	void hideCursor();
	void showCursor(const Component&);
	void centreCursor(const Component&, juce::MouseInputSource&);

	void appendRandomString(String&, Random&, int/*length*/,
		const String & /*legalChars*/ = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}