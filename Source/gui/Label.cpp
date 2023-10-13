#include "Label.h"

namespace gui
{
	Label::Label(Utils& u, const String& txt, const String& _tooltip) :
		Comp(u, _tooltip),
		just(Just::centred),
		font(font::flx()),
		text(txt),
		col(juce::Colours::white)
	{
		if (tooltip.isEmpty())
			setInterceptsMouseClicks(false, true);
	}

	bool Label::isEmpty() const noexcept
	{
		return text.isEmpty();
	}

	bool Label::isNotEmpty() const noexcept
	{
		return !isEmpty();
	}

	void Label::setText(const String& txt)
	{
		if (txt == text)
			return;

		text = txt;
	}

	void Label::paint(Graphics& g)
	{
		g.setFont(font);
		g.setColour(col);
		g.drawText(text, getLocalBounds().toFloat(), just, false);
	}

	void Label::setHeight(float h)
	{
		font.setHeight(h);
	}

	float Label::getMaxHeight() const noexcept
	{
		const auto w = static_cast<float>(getWidth());
		const auto h = static_cast<float>(getHeight());
		return findMaxHeight(font, text, w, h);
	}

	void Label::setMaxHeight() noexcept
	{
		setHeight(getMaxHeight());
	}

	float findMaxCommonHeight(const Label* labels, int size) noexcept
	{
		auto maxHeight = labels[0].getMaxHeight();
		for (auto i = 1; i < size; ++i)
			maxHeight = std::min(labels[i].getMaxHeight(), maxHeight);
		return maxHeight;
	}

	void setMaxCommonHeight(Label* labels, int size) noexcept
	{
		const auto h = findMaxCommonHeight(labels, size);
		for (auto i = 0; i < size; ++i)
			labels[i].setHeight(h);
	}

	void setMaxCommonHeight(Label* labels, size_t size) noexcept
	{
		setMaxCommonHeight(labels, static_cast<int>(size));
	}
}