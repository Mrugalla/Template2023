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
			Label(u),
			Label(u)
		}
	{
		layout.init
		(
			{ 34, 1, 8 },
			{ 1 }
		);

		makeTextLabel(labels[kTooltip], "", font::dosisLight(), Just::bottomLeft, CID::Txt, "Read the tooltips while hovering GUI elements to find out more about them!");
		makeTextLabel(labels[kBuildDate], getBuildDate(), font::dosisLight(), Just::bottomRight, CID::Txt, "The version of this plugin is defined by the time it was built.");

		for (auto& label : labels)
			addAndMakeVisible(label);

		addEvt([&tt = labels[kTooltip]](evt::Type type, const void* stf)
		{
			if (type == evt::Type::TooltipUpdated)
			{
				const auto str = static_cast<const String*>(stf);
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