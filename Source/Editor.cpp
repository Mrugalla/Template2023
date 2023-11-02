#include "Editor.h"

namespace gui
{
    evt::Evt makeEvt(Component& comp)
    {
        return [&c = comp](evt::Type type, const void*)
        {
            switch (type)
            {
            case evt::Type::ColourSchemeChanged:
                return repaintWithChildren(&c);
            case evt::Type::ClickedEmpty:
                return c.giveAwayKeyboardFocus();
            }
        };
    }

    Editor::Editor(Processor& p) :
        AudioProcessorEditor(&p),
        audioProcessor(p),
        utils(*this, p),
        layout(),
        evtMember(utils.eventSystem, makeEvt(*this)),
        bgImage(utils),
        tooltip(utils),
        labels
        {
            Label(utils, false),
            Label(utils, false)
        },
        filterTypeButton(utils),

        painterMacro(true),
        painterGainOut(false),
        painterPB(false),
        painterSlew(),

        macroKnob(utils),
        slewKnob(utils),
        gainOutKnob(utils),
        pbKnob(utils)
    {
        layout.init
        (
            { 1, 3, 8, 3, 3, 1 },
            { 1, 2, 5, 2, 2, 1 }
        );

        addAndMakeVisible(bgImage);
        addAndMakeVisible(bgImage.refreshButton);
        addAndMakeVisible(tooltip);

        makeTextLabel(labels[kDev], JucePlugin_Manufacturer, font::flx(), Just::centred, CID::Txt);
        makeTextLabel(labels[kTitle], JucePlugin_Name, font::flx(), Just::centred, CID::Txt);
        for (auto& label : labels)
            addAndMakeVisible(label);

        makeParameter(filterTypeButton, PID::FilterType, Button::Type::kChoice, "Type");
        addAndMakeVisible(filterTypeButton);

        addAndMakeVisible(macroKnob);
        macroKnob.attachParameter("Macro", PID::Macro, &painterMacro);
        addAndMakeVisible(slewKnob);
        slewKnob.attachParameter("Slew", PID::Slew, &painterSlew);
        addAndMakeVisible(gainOutKnob);
        gainOutKnob.attachParameter("Gain Out", PID::GainOut, &painterGainOut);
        addAndMakeVisible(pbKnob);
		pbKnob.attachParameter("PB Range", PID::PitchbendRange, &painterPB);

        const auto& user = *audioProcessor.state.props.getUserSettings();
        const auto editorWidth = user.getDoubleValue("EditorWidth", EditorWidth);
        const auto editorHeight = user.getDoubleValue("EditorHeight", EditorHeight);
        setOpaque(true);
        setResizable(true, true);
        setSize
        (
            static_cast<int>(editorWidth),
            static_cast<int>(editorHeight)
        );
    }
    
    void Editor::paintOverChildren(Graphics& g)
    {
        layout.paint(g, getColour(CID::Hover));
    }

    void Editor::resized()
    {
        if (getWidth() < EditorMinWidth)
            return setSize(EditorMinWidth, getHeight());
        if (getHeight() < EditorMinHeight)
            return setSize(getWidth(), EditorMinHeight);

        utils.resized();
        layout.resized(getLocalBounds());

        bgImage.setBounds(getLocalBounds());
        bgImage.refreshButton.setBounds(layout.cornerTopRight().toNearestInt());

        tooltip.setBounds(layout.bottom().toNearestInt());

        layout.place(labels[kDev], 1, 1, 1, 1);
        layout.place(labels[kTitle], 2, 1, 1, 1);
        setMaxCommonHeight(labels.data(), kNumLabels);

        layout.place(filterTypeButton, 2, 3, 1, 1, true);
        filterTypeButton.label.setMaxHeight();

        layout.place(macroKnob, 1, 2, 1, 2, false);
        layout.place(slewKnob, 2, 2, 1, 1, false);
        layout.place(gainOutKnob, 3, 2, 1, 2, false);
        layout.place(pbKnob, 4, 2, 1, 2, false);

        layout.place(macroKnob.label, 1, 4, 1, 1, false);
        layout.place(slewKnob.label, 2, 4, 1, 1, false);
        layout.place(gainOutKnob.label, 3, 4, 1, 1, false);
        layout.place(pbKnob.label, 4, 4, 1, 1, false);

        auto& user = *audioProcessor.state.props.getUserSettings();
		const auto editorWidth = static_cast<double>(getWidth());
		const auto editorHeight = static_cast<double>(getHeight());
		user.setValue("EditorWidth", editorWidth);
		user.setValue("EditorHeight", editorHeight);
	}

    void Editor::mouseEnter(const Mouse&)
    {
        evtMember(evt::Type::TooltipUpdated);
    }

    void Editor::mouseUp(const Mouse&)
    {
        evtMember(evt::Type::ClickedEmpty);
    }
}