#include "Comp.h"

namespace gui
{
	Comp::Comp(Utils& u, const String& _tooltip) :
		utils(u),
		layout(),
		tooltip(_tooltip),
		members(),
		callbacks()
	{
	}

	Comp::~Comp()
	{
		for (auto& cb : callbacks)
			utils.removeCallback(&cb);
	}
	
	void Comp::initLayout(const std::vector<int>& xL, const std::vector<int>& yL)
	{
		layout.init(xL, yL);
	}

	void Comp::initLayout(const String& xL, const String& yL)
	{
		layout.fromStrings(xL, yL);
	}

	void Comp::addEvt(const evt::Evt& evt)
	{
		members.push_back({ utils.eventSystem, evt });
	}

	void Comp::mouseEnter(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::TooltipUpdated, &tooltip);
	}

	void Comp::mouseUp(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::ClickedEmpty, this);
	}

	void Comp::addCallback(const Callback& callback, cbFPS fps)
	{
		removeCallbacks(callback.id);
		callbacks.push_back(callback);
		utils.addCallback(&callbacks.back(), fps);
	}

	void Comp::popCallback()
	{
		utils.removeCallback(&callbacks.back());
		callbacks.pop_back();
	}

	void Comp::removeCallbacks(int id)
	{
		for (auto i = 0; i < callbacks.size(); ++i)
			if (callbacks[i].id == id)
			{
				utils.removeCallback(&callbacks[i]);
				callbacks.erase(callbacks.begin() + i);
				return removeCallbacks(id);
			}
	}
}
