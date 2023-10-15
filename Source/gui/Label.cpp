#include "Label.h"

namespace gui
{
	Label::Label(Utils& u) :
		Comp(u),
		text(""),
		font(),
		just(Just::centred),
		path(),
		stroke(utils.thicc),
		img(),
		col(juce::Colours::white),
		type(Type::NumTypes)
	{
		setInterceptsMouseClicks(false, false);
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
		g.setColour(col);

		switch (type)
		{
		case Type::Text:
			g.setFont(font);
			g.drawText(text, getLocalBounds().toFloat(), just, false);
			break;
		case Type::Path:
			g.strokePath(path, stroke);
			break;
		case Type::Image:
			g.drawImage(img, getLocalBounds().toFloat(), RectPlacement::Flags::centred, true);
			break;
		}
	}

	void Label::setHeight(float h) noexcept
	{
		if (h == 0.f)
			return;
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

	void Label::resized()
	{
		stroke.setStrokeThickness(utils.thicc);
	}

	//////

	void makeTextLabel(Label& label, const String& text, const Font& font, Just just, Colour col, const String& tooltip)
	{
		label.type = Label::Type::Text;
		label.setText(text);
		label.font = font;
		label.just = just;
		label.col = col;
		label.setTooltip(tooltip);
	}

	void makePathLabel(Label& label, const Path& path, Stroke::JointStyle joint, Stroke::EndCapStyle end, Colour col, const String& tooltip)
	{
		label.type = Label::Type::Path;
		label.path = path;
		label.stroke.setStrokeThickness(label.utils.thicc);
		label.stroke.setJointStyle(joint);
		label.stroke.setEndStyle(end);
		label.col = col;
		label.setTooltip(tooltip);
	}

	void makeImageLabel(Label& label, const Image& img, Colour col, const String& tooltip)
	{
		label.type = Label::Type::Image;
		label.img = img;
		label.col = col;
		label.setTooltip(tooltip);
	}

	//////

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