#pragma once
#include "Comp.h"

namespace gui
{
	struct Label :
		public Comp
	{
		enum class Type { Text, Path, Image, NumTypes };
		static constexpr int NumTypes = static_cast<int>(Type::NumTypes);

		/* u */
		Label(Utils&);

		bool isEmpty() const noexcept;

		bool isNotEmpty() const noexcept;

		void setText(const String&);

		void paint(Graphics&) override;

		void setHeight(float) noexcept;

		float getMaxHeight() const noexcept;

		void setMaxHeight() noexcept;

		void resized() override;

		String text;
		Font font;
		Just just;
		Path path;
		Stroke stroke;
		Image img;
		Colour col;
		Type type;
	};

	//////

	/* label, text, font, just, col, tooltip */
	void makeTextLabel(Label&, const String&, const Font&, Just, Colour, const String& = "");

	/* label, path, jointStyle, endStyle, col, tooltip */
	void makePathLabel(Label&, const Path&, Stroke::JointStyle, Stroke::EndCapStyle, Colour, const String& = "");

	/* label, image, col, tooltip */
	void makeImageLabel(Label&, const Image&, Colour, const String& = "");

	//////

	/* labels, size */
	float findMaxCommonHeight(const Label*, int) noexcept;

	/* labels, size */
	void setMaxCommonHeight(Label*, int) noexcept;

	/* labels, size */
	void setMaxCommonHeight(Label*, size_t) noexcept;
}