#pragma once
#include "Utils.h"
#include "Layout.h"

namespace gui
{
	struct Comp :
		public Component
	{
		/* utils, tooltip */
		Comp(Utils&, const String & = "");

		~Comp();

		/* xL, yL */
		void initLayout(const std::vector<int>&, const std::vector<int>&);

		/* xL, yL */
		void initLayout(const String&, const String&);

		Utils& utils;
		Layout layout;
		String tooltip;
		std::vector<evt::Member> members;
		Callbacks callbacks;

		void paint(Graphics&) override;

		void mouseEnter(const Mouse&) override;

		void mouseUp(const Mouse&) override;
	};
}