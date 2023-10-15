#include "Tooltip.h"

namespace gui
{
	String getBuildDate()
	{
		return static_cast<String>(JucePlugin_Manufacturer) + ", v: " + static_cast<String>(__DATE__) + " " + static_cast<String>(__TIME__);
	}

	Tooltip::Tooltip(Utils& u) :
		Comp(u),
		labels
		{
			Label(u, "", "Read the tooltips while hovering GUI elements to find out more about them!"),
			Label(u, getBuildDate(), "The version of this plugin is defined by the time it was built.")
		}
	{
		layout.init
		(
			{ 34, 1, 8 },
			{ 1 }
		);

		labels[kTooltip].font = font::dosisLight();
		labels[kBuildDate].font = font::dosisLight();

		for (auto& label : labels)
			addAndMakeVisible(label);

		labels[kTooltip].just = Just::bottomLeft;
		labels[kBuildDate].just = Just::bottomRight;

		addEvt([&tt = labels[kTooltip]](evt::Type type, const void* stuff)
		{
			if (type == evt::Type::TooltipUpdated)
			{
				const auto str = static_cast<const String*>(stuff);
				tt.setText(str == nullptr ? "" : *str);
				tt.setMaxHeight();
				tt.repaint();
			}
		});
	}

	void Tooltip::resized()
	{
		layout.resized(getLocalBounds());

		layout.place(labels[kTooltip], 0, 0, 1, 1, false);
		layout.place(labels[kBuildDate], 2, 0, 1, 1, false);

		for(auto& label: labels)
			label.setMaxHeight();
	}
}

//