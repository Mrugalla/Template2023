#include "Comp.h"

namespace gui
{
	Comp::Comp(Utils& _utils, const String& _tooltip) :
		utils(_utils),
		layout(),
		tooltip(_tooltip)//,
		//evts()
	{
		//evts.reserve(1);
		//evts.emplace_back(utils.getEventSystem(), makeNotifyBasic(this));
	}
	
	void Comp::initLayout(const std::vector<int>& xL, const std::vector<int>& yL)
	{
		layout.init(xL, yL);
	}

	void Comp::initLayout(const String& xL, const String& yL)
	{
		layout.fromStrings(xL, yL);
	}

	//void Comp::notify(EvtType type, const void* stuff)
	//{
	//	evts[0](type, stuff);
	//}

	void Comp::paint(Graphics& g)
	{
		g.setColour(juce::Colour(0xffff0099));
		g.drawRect(getLocalBounds().toFloat(), 1.f);
	}

	void Comp::mouseEnter(const Mouse&)
	{
		//notify(EvtType::TooltipUpdated, &tooltip);
	}

	void Comp::mouseUp(const Mouse&)
	{
		//notify(EvtType::ClickedEmpty, this);
	}
}
