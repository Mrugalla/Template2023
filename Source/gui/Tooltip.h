#pragma once
#include "Label.h"

namespace gui
{
	inline String getBuildDate()
	{
		return static_cast<String>(JucePlugin_Manufacturer) + ", v: " + static_cast<String>(__DATE__) + " " + static_cast<String>(__TIME__);
	}

	struct Tooltip :
		public Comp
	{
		Tooltip(Utils& u) :
			Comp(u),
			labels
			{
				Label(u, "", "Yes, I am the tooltip text. Read me while hovering parameters and stuff!"),
				Label(u, getBuildDate(), "The version of this plugin is defined by the time I built it.")
			}
		{
			layout.init
			(
				{ 1, 3 },
				{ 1 }
			);

			for (auto& label : labels)
			{
				addAndMakeVisible(label);
				label.font = font::dosisRegular();
			}
			
			addEvt([&tt = labels[kTooltip]](evt::Type type, const void* stuff)
			{
				if (type == evt::Type::TooltipUpdated)
				{
					const auto str = static_cast<const String*>(stuff);
					tt.setText(str == nullptr ? "" : *str);
					tt.repaint();
				}
			});
		}

		void resized() override
		{
			layout.resized(getLocalBounds());

			for(auto i = 0; i < kNumLabels; ++i)
				layout.place(labels[i], i, 0, 1, 1, false);

			//setMaxCommonHeight(labels.data(), labels.size());
		}

	protected:
		enum { kBuildDate, kTooltip, kNumLabels };
		std::array<Label, kNumLabels> labels;
	};
}