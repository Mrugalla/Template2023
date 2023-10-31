#pragma once
#include "Button.h"

namespace gui
{
	struct Knob :
		public Comp
	{
        static constexpr float SensitiveDrag = .2f;
        static constexpr float WheelSpeed = .02f;

		using Func = std::function<void()>;
		using OnDrag = std::function<void(const PointF&, const Mouse&)>;
		using OnMouse = std::function<void(const Mouse&)>;
		using OnPaint = std::function<void(Graphics&)>;

		Knob(Utils& u) :
            Comp(u),
            onEnter([]() {}), onExit([]() {}), onDown([]() {}),
            onResize([]() {}), onDoubleClick([]() {}),
            onDrag([](const PointF&, const Mouse&) {}),
            onUp([](const Mouse&) {}), onWheel([](const Mouse&) {}),
            //label(u),
            dragXY(), lastPos(),
            //knobBounds(),
            //values(),
            hidesCursor(true)
		{
            //addAndMakeVisible(label);
        }

        void resized() override
        {
            layout.resized(getLocalBounds());
            onResize();
        }

        void mouseEnter(const Mouse& mouse) override
        {
            Comp::mouseEnter(mouse);
            onEnter();
        }

        void mouseExit(const Mouse& mouse) override
        {
            Comp::mouseExit(mouse);
			onExit();
        }

        void mouseDown(const Mouse& mouse) override
        {
            utils.giveDAWKeyboardFocus();
            
            dragXY.setXY
            (
                mouse.position.x,
                mouse.position.y
            );
            lastPos = getPosition().toFloat();

            onDown();
        }

        void mouseDrag(const Mouse& mouse) override
        {
            if (hidesCursor)
                hideCursor();

            const auto nPosition = getPosition().toFloat();
            const auto posShift = nPosition - lastPos;
            lastPos = nPosition;

            dragXY -= posShift;
            const auto shiftDown = mouse.mods.isShiftDown();
            const auto dragSpeed = shiftDown ? SensitiveDrag : 1.f;
            const auto dragOffset = (mouse.position - dragXY) * dragSpeed;
            onDrag(dragOffset, mouse);
            dragXY = mouse.position;
        }

        void mouseUp(const Mouse& mouse) override
        {
            onUp(mouse);

            if (hidesCursor && mouse.mouseWasDraggedSinceMouseDown())
                showCursor(*this);
        }

        void mouseWheelMove(const Mouse& mouse, const MouseWheel& wheel) override
        {
            if (mouse.mods.isAnyMouseButtonDown())
                return;

            const bool reversed = wheel.isReversed ? -1.f : 1.f;
            const bool isTrackPad = wheel.deltaY * wheel.deltaY < .0549316f;
            if (isTrackPad)
                dragXY.setXY
                (
                    reversed * wheel.deltaX,
                    reversed * wheel.deltaY
                );
            else
            {
                const auto deltaXPos = wheel.deltaX > 0.f ? 1.f : -1.f;
                const auto deltaYPos = wheel.deltaY > 0.f ? 1.f : -1.f;
                dragXY.setXY
                (
                    reversed * WheelSpeed * deltaXPos,
                    reversed * WheelSpeed * deltaYPos
                );
            }

            if (mouse.mods.isShiftDown())
                dragXY *= SensitiveDrag;

            onWheel(mouse);
        }

        void mouseDoubleClick(const Mouse& mouse) override
        {
            Comp::mouseDoubleClick(mouse);
            onDoubleClick();
        }

        Func onEnter, onExit, onDown, onResize, onDoubleClick;
        OnDrag onDrag;
        OnMouse onUp, onWheel;
        //Label label;
        PointF dragXY, lastPos;
        //std::vector<float> values;
        bool hidesCursor;
	};

    struct ModDial :
        public Knob
    {
        static constexpr float NumStepsPaintCurve = 13.f;

        ModDial(Utils& u) :
            Knob(u),
            prms(),
            biasBounds(),
            path()
        {
            onResize = [&]()
            {
                auto thicc = utils.thicc * Tau;
                biasBounds = getLocalBounds().toFloat().reduced(thicc);
            };

            onDrag = [&](const PointF& dragOffset, const Mouse& mouse)
            {
                const auto speed = 1.f / utils.getDragSpeed();
                const auto dragXY = dragOffset * speed;

                if (mouse.mods.isRightButtonDown())
                    for (auto prm : prms)
                        prm->setModBias(prm->getModBias() - dragXY.y);
                else
                    for (auto prm : prms)
                        prm->setModDepth(prm->getModDepth() - dragXY.y);
            };

            onUp = [&](const Mouse& mouse)
            {
                if (mouse.mouseWasDraggedSinceMouseDown())
                    return;

                if (mouse.mods.isAltDown())
                    for (auto prm : prms)
                        prm->setModulationDefault();
            };

            onDoubleClick = [&]()
            {
                for (auto prm : prms)
                    prm->setModulationDefault();
            };
        }

        void attachParameters(PID* pIDs, int numPIDs)
        {
            prms.clear();
            prms.reserve(numPIDs);
            for (auto i = 0; i < numPIDs; ++i)
                prms.emplace_back(&utils.getParam(pIDs[i]));
        }

        void paint(Graphics& g) override
        {
            g.setColour(getColour(CID::Mod));
            g.fillEllipse(getLocalBounds().toFloat());

            path.clear();

            const auto left = biasBounds.getX();
            const auto btm = biasBounds.getBottom();
            const auto right = biasBounds.getRight();
            const auto top = biasBounds.getY();
            const auto width = biasBounds.getWidth();
            path.startNewSubPath(left, btm);

            const auto numStepsInv = 1.f / NumStepsPaintCurve;
            const auto bias = prms[0]->getModBias();
            for (auto i = 0.f; i < NumStepsPaintCurve; ++i)
            {
                const auto ratio = i * numStepsInv;
                const auto x = left + ratio * width;
                const auto y = Param::biased(btm, top, bias, ratio);
                path.lineTo(x, y);
            }
            
            path.lineTo(right, top);

            Stroke stroke(utils.thicc * Pi, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);
            g.setColour(getColour(CID::Bg));
            g.strokePath(path, stroke);
        }

        std::vector<Param*> prms;
        BoundsF biasBounds;
        Path path;
    };

    struct KnobParam :
        public Knob
    {
        enum CBTypes { kUpdateParameterCB };
        enum ValTypes { Value, ValMod, ModDepth, ModBias, NumValTypes };
        using OnPaint = std::function<void(const KnobParam&, Graphics&)>;

        KnobParam(Utils& u) :
            Knob(u),
            values{ 0.f, 0.f, 0.f, 0.f },
            prms(),
            onPaint([](const KnobParam&, Graphics&) {}),
            modDial(u),
            lockButton(u),
            label(u),
            knobBounds()
        {
		    addAndMakeVisible(label);
            label.cID = CID::Txt;
            label.font = font::dosisLight();

            addAndMakeVisible(lockButton);
            makeTextButton(lockButton, "L", "Click here to lock this parameter.", CID::Interact);
            lockButton.onClick = [&](const Mouse&)
			{
                for (auto prm : prms)
                    prm->switchLock();
			};
        }

        void attachParameters(const String& name, PID* pIDs, int numPIDs)
        {
            prms.clear();
            prms.reserve(numPIDs);
            for (auto i = 0; i < numPIDs; ++i)
                prms.emplace_back(&utils.getParam(pIDs[i]));

            setTooltip(toTooltip(pIDs[0]));
            setName(name);

            makeTextLabel(label, name, font::dosisBold(), Just::centred, CID::Txt, "arr");
            auto& mainParam = utils.getParam(pIDs[0]);

            const auto drawValToLabelFunc = [&, &prm = mainParam]()
                {
                    label.setText(prm.getCurrentValueAsText());
                    label.setMaxHeight();
                    label.repaint();
                };

            onEnter = [drawValToLabelFunc]()
            {
                drawValToLabelFunc();
            };

            onExit = [&]()
            {
                label.setText(getName());
                label.setMaxHeight();
                label.repaint();
            };

            onDown = [&, drawValToLabelFunc]()
            {
                for (auto prm : prms)
                    if (!prm->isInGesture())
                        prm->beginGesture();

                drawValToLabelFunc();
            };

            onDrag = [&, drawValToLabelFunc](const PointF& dragOffset, const Mouse&)
            {
                const auto speed = 1.f / utils.getDragSpeed();
                const auto dragVal = -dragOffset.y * speed;

                for (auto prm : prms)
                {
                    const auto newValue = juce::jlimit(0.f, 1.f, prm->getValue() + dragVal);
                    prm->setValueFromEditor(newValue);
                }

                drawValToLabelFunc();
            };

            onUp = [&, drawValToLabelFunc](const Mouse& mouse)
            {
                if (!mouse.mouseWasDraggedSinceMouseDown())
                    if (mouse.mods.isAltDown())
                        for (auto prm : prms)
                            prm->setValueFromEditor(prm->getDefaultValue());

                for (auto prm : prms)
                    prm->endGesture();

                drawValToLabelFunc();
            };

            onWheel = [&, drawValToLabelFunc](const Mouse&)
            {
                for (auto prm : prms)
                {
                    const auto& range = prm->range;
                    const auto interval = range.interval;

                    if (interval > 0.f)
                    {
                        const auto nStep = interval / range.getRange().getLength();
                        dragXY.setY(dragXY.y > 0.f ? nStep : -nStep);
                        auto newValue = juce::jlimit(0.f, 1.f, prm->getValue() + dragXY.y);
                        newValue = range.convertTo0to1(range.snapToLegalValue(range.convertFrom0to1(newValue)));
                        prm->setValueWithGesture(newValue);
                    }
                    else
                    {
                        const auto newValue = juce::jlimit(0.f, 1.f, prm->getValue() + dragXY.y);
                        prm->setValueWithGesture(newValue);
                    }
                }

                drawValToLabelFunc();
            };

            onDoubleClick = [&, drawValToLabelFunc]()
            {
                for (auto prm : prms)
                    if (!prm->isInGesture())
                        prm->setValueWithGesture(prm->getDefaultValue());

                drawValToLabelFunc();
            };

            bool isMacro = pIDs[0] == PID::Macro;
            if (!isMacro)
            {
                addAndMakeVisible(modDial);
                modDial.attachParameters(pIDs, numPIDs);
            }

            add(Callback([&, &prm = mainParam]()
            {
                bool shallRepaint = false;

                setLocked(prm.isLocked());

                const auto vn = prm.getValue();
                const auto md = prm.getModDepth();
                const auto vm = prm.getValMod();
                const auto mb = prm.getModBias();

                auto& vals = values;

                if (vals[Value] != vn || vals[ModDepth] != md || vals[ValMod] != vm || vals[ModBias] != mb)
                {
                    vals[Value] = vn;
                    vals[ModDepth] = md;
                    vals[ValMod] = vm;
                    vals[ModBias] = mb;
                    shallRepaint = true;
                }

                if (shallRepaint)
                    repaint();
            }, kUpdateParameterCB, cbFPS::k60, true));

            onResize = [&]()
            {
                const auto thicc = utils.thicc;

                knobBounds = layout(0, 0, 3, 2, true).reduced(thicc);
                layout.place(label, 0, 2, 3, 1, false);
                label.setMaxHeight();

                if (modDial.isVisible())
                    layout.place(modDial, 1, 1, 1, 1, true);
                layout.place(lockButton, 1.5f, 1.5f, 1.5f, 1.5f, true);
            };

            layout.init
            (
                { 1, 1, 1 },
                { 13, 5, 5 }
            );

            const auto angleWidth = PiQuart * 3.f;
            const auto angleRange = angleWidth * 2.f;

            onPaint = [isMacro, angleWidth, angleRange](const KnobParam& k, Graphics& g)
            {
                const auto& vals = k.values;
                const auto thicc = k.utils.thicc;
                const auto thicc2 = thicc * 2.f;
                const auto thicc3 = thicc * 3.f;
                const auto thicc5 = thicc * 5.f;
                Stroke strokeType(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::butt);
                const auto radius = k.knobBounds.getWidth() * .5f;
                const auto radiusInner = radius * .8f;
                const auto radDif = (radius - radiusInner) * .8f;

                PointF centre
                (
                    radius + k.knobBounds.getX(),
                    radius + k.knobBounds.getY()
                );

                auto col = getColour(CID::Txt);

                { // paint lines

                    Path arcOutline;
                    arcOutline.addCentredArc
                    (
                        centre.x, centre.y,
                        radius, radius,
                        0.f,
                        -angleWidth, angleWidth,
                        true
                    );
                    g.setColour(col);
                    g.strokePath(arcOutline, strokeType);

                    Path arcInline;
                    arcInline.addCentredArc
                    (
                        centre.x, centre.y,
                        radiusInner, radiusInner,
                        0.f,
                        -angleWidth, angleWidth,
                        true
                    );
                    Stroke stroke2 = strokeType;
                    stroke2.setStrokeThickness(radDif);
                    g.strokePath(arcInline, stroke2);
                };

                const auto valNormAngle = vals[Value] * angleRange;
                const auto valAngle = -angleWidth + valNormAngle;
                const auto radiusExt = radius + thicc;

                // paint modulation
                if (!isMacro)
                {
                    const auto valModAngle = vals[ValMod] * angleRange;
                    const auto modAngle = -angleWidth + valModAngle;
                    const auto modTick = LineF::fromStartAndAngle(centre, radiusExt, modAngle);
                    const auto shortenedModTick = modTick.withShortenedStart(radiusInner - thicc);

                    g.setColour(Colours::c(CID::Bg));
                    g.drawLine(shortenedModTick, thicc * 4.f);

                    const auto maxModDepthAngle = juce::jlimit(-angleWidth, angleWidth, valNormAngle + vals[ModDepth] * angleRange - angleWidth);
                    
                    g.setColour(Colours::c(CID::Mod));
                    g.drawLine(modTick.withShortenedStart(radiusInner), thicc2);
                    {
                        Path modPath;
                        modPath.addCentredArc
                        (
                            centre.x, centre.y,
                            radius, radius,
                            0.f,
                            maxModDepthAngle, valAngle,
                            true
                        );
                        g.strokePath(modPath, strokeType);
                    }
                };

                col = Colours::c(CID::Interact);

                { // paint tick
                    const auto tickLine = LineF::fromStartAndAngle(centre, radius, valAngle);
                    const auto shortened = tickLine.withShortenedStart(radiusInner - thicc);
                    g.setColour(Colours::c(CID::Bg));
                    g.drawLine(shortened, thicc5);
                    g.setColour(col);
                    g.drawLine(shortened, thicc3);
                }
            };
        }

        void attachParameter(const String& name, PID pID)
		{
			attachParameters(name, &pID, 1);
		}

        void paint(Graphics& g) override
        {
            onPaint(*this, g);
        }

        std::array<float, NumValTypes> values;
        std::vector<Param*> prms;
        OnPaint onPaint;
        ModDial modDial;
        Button lockButton;
        Label label;
        BoundsF knobBounds;
    };
}