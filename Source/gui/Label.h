#pragma once
#include "Comp.h"

namespace gui
{
	struct Label :
		public Comp
	{
		Label(Utils& u, const String& txt = "", const String& _tooltip = "") :
			Comp(u, _tooltip),
			just(Just::centred),
			font(font::flx()),
			text(txt),
			col(juce::Colours::white)
		{
			if (tooltip.isEmpty())
				setInterceptsMouseClicks(false, true);
		}

		bool isEmpty() const noexcept
		{
			return text.isEmpty();
		}

		bool isNotEmpty() const noexcept
		{
			return !isEmpty();
		}

		void setText(const String& txt)
		{
			if (txt == text)
				return;

			text = txt;
		}

		void paint(Graphics& g)
		{
			g.setFont(font);
			g.setColour(col);
			g.drawText(text, getLocalBounds().toFloat(), just, false);
		}

		void setHeight(float h)
		{
			font.setHeight(h);
			/*
			const auto thicc = utils.thicc;
			const auto width = static_cast<float>(getWidth());
			const auto height = static_cast<float>(getHeight());

			const auto fontBounds = boundsOf(font, text);

			if (fontBounds.getWidth() != 0.f)
			{
				const PointF dif
				(
					fontBounds.getWidth() - width,
					fontBounds.getHeight() - height
				);

				float ratio;
				if (dif.x > dif.y)
					ratio = width / fontBounds.getWidth();
				else
					ratio = height / fontBounds.getHeight();

				font.setHeight(font.getHeight() * ratio - thicc);
			}
			*/
		}

		float getMaxHeight() const noexcept
		{
			const auto w = static_cast<float>(getWidth());
			const auto h = static_cast<float>(getHeight());
			return findMaxHeight(font, text, w, h);
		}

		Just just;
		Font font;
		String text;
		Colour col;
	};

	inline float findMaxCommonHeight(const Label* labels, int size) noexcept
	{
		auto maxHeight = labels[0].getMaxHeight();
		for (auto i = 1; i < size; ++i)
		{
			const auto h = labels[i].getMaxHeight();
			if (h < maxHeight)
				maxHeight = h;
		}
		return maxHeight;
	}

	inline void setMaxCommonHeight(Label* labels, int size) noexcept
	{
		auto h = findMaxCommonHeight(labels, size);
		for(auto i = 0; i < size; ++i)
			labels[i].setHeight(h);
	}
}