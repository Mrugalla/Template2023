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

		/* xL, yL */
		void initLayout(const std::vector<int>&, const std::vector<int>&);

		/* xL, yL */
		void initLayout(const String&, const String&);

		//void notify(EvtType, const void* = nullptr);

		Utils& utils;
		Layout layout;
		String tooltip;
	protected:
		//std::vector<Evt> evts;

		void paint(Graphics&) override;

		void mouseEnter(const Mouse&) override;

		void mouseUp(const Mouse&) override;

	private:
		//Notify makeNotifyBasic(Comp*);

	};
}