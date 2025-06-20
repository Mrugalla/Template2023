#include "Prompt.h"

namespace gui
{
	Prompt::Prompt(Utils& u) :
		Comp(u),
		message(u),
		buttons()
	{
		layout.init
		(
			{ 1 },
			{ 3, 2 }
		);

		addEvt([&](evt::Type t, const void* stuff)
		{
			if (t == evt::Type::PromptActivate)
			{
				const auto data = reinterpret_cast<const PromptData*>(stuff);
				if (data == nullptr)
					return;
				activate(*data);
			}
			else if (t == evt::Type::PromptDeactivate)
			{
				deactivate();
			}
		});

		addAndMakeVisible(message);
	}

	void Prompt::paint(Graphics& g)
	{
		setCol(g, CID::Darken);
		const auto thicc = utils.thicc * 4.f;
		const auto b = getLocalBounds().toFloat().reduced(thicc);
		g.fillRoundedRectangle(b, thicc);
	}

	void Prompt::resized()
	{
		Comp::resized();
		if (buttons.size() == 0)
			return;
		layout.place(message, 0, 0, 1, 1);
		message.setMaxHeight(utils.thicc * 2.f);
		auto buttonsBounds = layout(0, 1, 1, 1);
		const auto y = buttonsBounds.getY();
		const auto w = buttonsBounds.getWidth();
		const auto h = buttonsBounds.getHeight();
		auto x = buttonsBounds.getX();
		const auto inc = w / static_cast<float>(buttons.size());
		for (auto& b : buttons)
		{
			b->setBounds(BoundsF(x, y, inc, h).toNearestInt());
			x += inc;
		}
	}

	void Prompt::activate(const PromptData& data)
	{
		for (const auto& button : buttons)
			removeChildComponent(button.get());
		makeTextLabel(message, data.message, font::text(), Just::centred, CID::Txt);
		buttons.clear();
		for (const auto& d : data.buttons)
		{
			auto b = std::make_unique<Button>(utils);
			auto& btn = *b;
			makeTextButton(btn, d.text, d.tooltip, CID::Interact);
			btn.onClick = [oc = d.onClick](const Mouse&) { oc(); };
			buttons.push_back(std::move(b));
		}
		for (auto& button : buttons)
			addAndMakeVisible(*button);
		setVisible(true);
		resized();
	}

	void Prompt::deactivate()
	{
		setVisible(false);
	}
}