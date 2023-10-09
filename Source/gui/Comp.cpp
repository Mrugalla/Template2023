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

	void Comp::paint(Graphics& g)
	{
		g.setColour(juce::Colour(0xffff0099));
		g.drawRect(getLocalBounds().toFloat(), 1.f);
	}

	void Comp::mouseEnter(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::TooltipUpdated, &tooltip);
	}

	void Comp::mouseUp(const Mouse&)
	{
		utils.eventSystem.notify(evt::Type::ClickedEmpty, this);
	}
}
