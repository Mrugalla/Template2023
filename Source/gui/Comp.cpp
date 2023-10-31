#include "Comp.h"

namespace gui
{
	Comp::Comp(Utils& u, const String& _tooltip) :
		utils(u),
		layout(),
		tooltip(_tooltip),
		members(),
		callbacks()//,
		//stuff(),
		//comps()
	{
	}

	Comp::~Comp()
	{
		deregisterCallbacks();
	}
	
	void Comp::setLocked(bool e)
	{
		setAlpha(e ? LockAlpha : 1.f);
	}

	void Comp::deregisterCallbacks()
	{
		for (auto& cb : callbacks)
			utils.remove(&cb);
	}

	void Comp::add(const Callback& callback)
	{
		deregisterCallbacks();
		callbacks.push_back(callback);
		registerCallbacks();
	}

	void Comp::registerCallbacks()
	{
		for (auto& cb : callbacks)
			utils.add(&cb);
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

	void Comp::setTooltip(const String& t)
	{
		tooltip = t;
		if(tooltip.isNotEmpty())
			setInterceptsMouseClicks(true, true);
	}
}
