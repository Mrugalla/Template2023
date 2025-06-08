#pragma once
#include "Layout.h"
#include "Utils.h"
#include "Cursor.h"

namespace gui
{
	struct Comp :
		public Component
	{
		static constexpr float LockAlpha = .5f;
		static constexpr float AniLengthMs = 200.f;

		// utils
		Comp(Utils&);

		~Comp();

		void resized() override;

		// adding and removing components
		// child, visible
		void add(Comp&, bool = true);

		void remove(Comp&);

		void setLocked(bool);

		// callbacks
		void add(const Callback&);

		// layout
		// xL, yL
		void initLayout(const String&, const String&);

		// events
		void addEvt(const evt::Evt&);

		void notify(const evt::Type, const void* = nullptr);

		void mouseEnter(const Mouse&) override;

		void mouseUp(const Mouse&) override;

		void setTooltip(const String&);

		Utils& utils;
		Layout layout;
		String tooltip;
		std::vector<evt::Member> members;
		Callbacks callbacks;

		// events backend
		void deregisterCallbacks();

		void registerCallbacks();
	};
}