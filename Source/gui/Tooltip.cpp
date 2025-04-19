#include "Tooltip.h"

namespace gui
{
	String getBuildDate()
	{
		return static_cast<String>(__DATE__) + " " + static_cast<String>(__TIME__);
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

		auto& user = utils.getProps();
		const auto buildDateOldString = user.getValue("buildDate", "");
		const auto buildDateNewString = getBuildDate();
		
		const auto pluginVersion = 0;
		auto versionNumber = user.getDoubleValue("versionNumber", static_cast<double>(pluginVersion));
		const bool newBuild = buildDateOldString != buildDateNewString;
		
		if (newBuild)
		{
			if (static_cast<int>(versionNumber) != pluginVersion)
				versionNumber = static_cast<double>(pluginVersion);

#if !JUCE_DEBUG
			else
				versionNumber += .000001;
#endif
			user.setValue("versionNumber", versionNumber);
			user.setValue("buildDate", buildDateNewString);
		}
		
		const String versionNumberString = "v: " + String(versionNumber, 6);
		const String buildDateString = "This version's build date: " + buildDateNewString;
		makeTextLabel(labels[kBuildDate], versionNumberString, font::dosisLight(), Just::bottomRight, CID::Txt, buildDateString);

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