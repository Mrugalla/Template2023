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

		void addEvt(const evt::Evt&);

		void addCallback(const Callback&, cbFPS);

		void popCallback();

		void removeCallbacks(int id);

		void mouseEnter(const Mouse&) override;

		void mouseUp(const Mouse&) override;

		Utils& utils;
		Layout layout;
		String tooltip;
		std::vector<evt::Member> members;
		Callbacks callbacks;
	};
}