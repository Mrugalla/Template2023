#pragma once
#include "Comp.h"

namespace gui
{
	struct Label :
		public Comp
	{
		/* u, text, tooltip */
		Label(Utils&, const String & = "", const String & = "");

		bool isEmpty() const noexcept;

		bool isNotEmpty() const noexcept;

		void setText(const String&);

		void paint(Graphics&) override;

		void setHeight(float);

		float getMaxHeight() const noexcept;

		Just just;
		Font font;
		String text;
		Colour col;
	};

	/* labels, size */
	float findMaxCommonHeight(const Label*, int) noexcept;

	/* labels, size */
	void setMaxCommonHeight(Label*, int) noexcept;

	/* labels, size */
	void setMaxCommonHeight(Label*, size_t) noexcept;
}