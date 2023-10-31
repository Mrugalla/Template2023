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
		using OnDrag = std::function<void(const PointF&)>;
		using OnUp = std::function<void(const Mouse&)>;
		using OnPaint = std::function<void(Graphics&)>;

		enum class DragMode { Vertical, Horizontal, Both, NumDragModes };
		static constexpr int NumDragModes = static_cast<int>(DragMode::NumDragModes);

		Knob(Utils& u) :
            Comp(u),
            onEnter([]() {}), onExit([]() {}), onDown([]() {}), onWheel([]() {}),
            onResize([]() {}), onDoubleClick([]() {}),
            onDrag([](const PointF&) {}),
            onUp([](const Mouse&) {}),
            onPaint([](Graphics&) {}),
            label(u),
            dragXY(), lastPos(),
            knobBounds(),
            values(),
            hidesCursor(true),
            dragMode(DragMode::Vertical)
		{
            addAndMakeVisible(label);
        }

        void paint(Graphics& g) override
        {
            onPaint(g);
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
            if (mouse.mods.isRightButtonDown())
                return;

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
            if (mouse.mods.isRightButtonDown())
                return;

            if (hidesCursor)
                hideCursor();

            const auto nPosition = getPosition().toFloat();
            const auto posShift = nPosition - lastPos;
            lastPos = nPosition;

            dragXY -= posShift;
            const auto shiftDown = mouse.mods.isShiftDown();
            const auto dragSpeed = shiftDown ? SensitiveDrag : 1.f;
            const auto dragOffset = (mouse.position - dragXY) * dragSpeed;
            onDrag(dragOffset);
            dragXY = mouse.position;
        }

        void mouseUp(const Mouse& mouse) override
        {
            onUp(mouse);

            if (hidesCursor)
                if (mouse.mouseWasDraggedSinceMouseDown())
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

            onWheel();
        }

        void mouseDoubleClick(const Mouse& mouse) override
        {
            Comp::mouseDoubleClick(mouse);
            onDoubleClick();
        }

        Func onEnter, onExit, onDown, onWheel, onResize, onDoubleClick;
        OnDrag onDrag;
        OnUp onUp;
        OnPaint onPaint;
        Label label;
        PointF dragXY, lastPos;
        BoundsF knobBounds;
        std::vector<float> values;
        bool hidesCursor;
        DragMode dragMode;
	};

    inline void makeParameter(Knob& knob, PID* pIDs, const String& name, int numPIDs)
    {
        for(auto i = 0; i < numPIDs; ++i)
			knob.stuff.push_back(static_cast<int>(pIDs[i]));

        knob.setTooltip(toTooltip(pIDs[0]));
        knob.setName(name);
        makeTextLabel(knob.label, name, font::dosisBold(), Just::centred, CID::Txt, "arr");

        enum CBTypes { kUpdateParameterCB };
        enum ValTypes { Value, ValMod, ModDepth, ModBias, NumValTypes };

        knob.values.resize(NumValTypes, 0.f);
        auto& mainParam = knob.utils.getParam(pIDs[0]);

        const auto drawValToLabelFunc = [&k = knob, &prm = mainParam]()
        {
            k.label.setText(prm.getCurrentValueAsText());
            k.label.setMaxHeight();
            k.label.repaint();
        };

        knob.onEnter = [drawValToLabelFunc]()
        {
            drawValToLabelFunc();
        };

        knob.onExit = [&k = knob]()
        {
            k.label.setText(k.getName());
            k.label.setMaxHeight();
            k.label.repaint();
        };

        knob.onDown = [&k = knob, drawValToLabelFunc, numPIDs]()
        {
            for (auto i = 0; i < numPIDs; ++i)
            {
                const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                auto& prm = k.utils.getParam(pID);
                if (!prm.isInGesture())
                    prm.beginGesture();
            }
            
            drawValToLabelFunc();
        };

        knob.onDrag = [&k = knob, drawValToLabelFunc, numPIDs](const PointF& dragOffset)
        {
            const auto speed = 1.f / k.utils.getDragSpeed();
            const auto dragVal = (k.dragMode == Knob::DragMode::Vertical ? -dragOffset.y : dragOffset.x) * speed;

            for (auto i = 0; i < numPIDs; ++i)
            {
                const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                auto& prm = k.utils.getParam(pID);
                const auto newValue = juce::jlimit(0.f, 1.f, prm.getValue() + dragVal);
                prm.setValueFromEditor(newValue);
            }

            drawValToLabelFunc();
        };

        knob.onUp = [&k = knob, drawValToLabelFunc, numPIDs](const Mouse& mouse)
        {
            if (mouse.mods.isRightButtonDown())
                return;

            if (mouse.mouseWasDraggedSinceMouseDown())
                for(auto i = 0; i < numPIDs; ++i)
				{
					const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
					auto& prm = k.utils.getParam(pID);
					prm.endGesture();
				}
            else if (mouse.mods.isAltDown())
                for (auto i = 0; i < numPIDs; ++i)
                {
                    const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                    auto& prm = k.utils.getParam(pID);
                    prm.setValueFromEditor(prm.getDefaultValue());
                    prm.endGesture();
                }

            drawValToLabelFunc();
        };

        knob.onWheel = [&k = knob, drawValToLabelFunc, numPIDs]()
        {
            for (auto i = 0; i < numPIDs; ++i)
            {
                const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                auto& prm = k.utils.getParam(pID);

                const auto& range = prm.range;
                const auto interval = range.interval;

                if (interval > 0.f)
                {
                    const auto nStep = interval / range.getRange().getLength();
                    k.dragXY.setY(k.dragXY.y > 0.f ? nStep : -nStep);
                    auto newValue = juce::jlimit(0.f, 1.f, prm.getValue() + k.dragXY.y);
                    newValue = range.convertTo0to1(range.snapToLegalValue(range.convertFrom0to1(newValue)));
                    prm.setValueWithGesture(newValue);
                }
                else
                {
                    const auto newValue = juce::jlimit(0.f, 1.f, prm.getValue() + k.dragXY.y);
                    prm.setValueWithGesture(newValue);
                }
            }

            drawValToLabelFunc();
        };

        knob.onDoubleClick = [&k = knob, drawValToLabelFunc , numPIDs]()
        {
            for (auto i = 0; i < numPIDs; ++i)
            {
                const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                auto& prm = k.utils.getParam(pID);

                if (!prm.isInGesture())
                {
                    const auto dVal = prm.getDefaultValue();
                    prm.setValueWithGesture(dVal);
                }
            }

            drawValToLabelFunc();
        };

        bool isMacro = pIDs[0] == PID::Macro;

        std::unique_ptr<Knob> modDial = isMacro ? nullptr : std::make_unique<Knob>(knob.utils);
        if (modDial != nullptr)
        {
            auto& dial = *modDial;
            dial.stuff = knob.stuff;

            dial.onResize = [&k = dial]()
            {
                auto thicc = k.utils.thicc * 2.f;
                k.knobBounds = k.getLocalBounds().toFloat().reduced(thicc);
            };

            dial.onPaint = [&k = dial](Graphics& g)
            {
                g.setColour(getColour(CID::Mod));
                g.fillEllipse(k.getLocalBounds().toFloat());

                Path path;

                const auto left = k.knobBounds.getX();
                const auto btm = k.knobBounds.getBottom();
                path.startNewSubPath(left, btm);

                const auto right = k.knobBounds.getRight();
                const auto top = k.knobBounds.getY();
                path.lineTo(right, top);

                Stroke stroke(k.utils.thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);

                g.setColour(getColour(CID::Bg));
                g.strokePath(path, stroke);
            };

            dial.onDrag = [&k = dial, numPIDs](const PointF& dragOffset)
            {
                auto& utils = k.utils;
                const auto speed = 1.f / utils.getDragSpeed();
                const auto dragXY = dragOffset * speed;

                for (auto i = 0; i < numPIDs; ++i)
                {
                    const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                    auto& prm = utils.getParam(pID);
                    const auto newValue = prm.getModDepth() - dragXY.y;
                    prm.setModDepth(newValue);
                    /*
                    switch (state)
                    {
                    case StateMaxModDepth:
                        
                        break;
                    case StateModBias:
                        newValue = param->getModBias() - dragOffset.y;
                        param->setModBias(newValue);
                        break;
                    }
                    */
                }

            };

            dial.onUp = [&k = dial, numPIDs](const Mouse& mouse)
            {
                if (!mouse.mouseWasDraggedSinceMouseDown())
                {
                    if (mouse.mods.isCtrlDown())
                        for (auto i = 0; i < numPIDs; ++i)
                        {
                            const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                            auto& prm = k.utils.getParam(pID);
                            prm.setModulationDefault();
                        }
                }
            };

            dial.onDoubleClick = [&k = dial, numPIDs]()
            {
                for (auto i = 0; i < numPIDs; ++i)
                {
                    const auto pID = static_cast<PID>(static_cast<int>(k.stuff[i]));
                    auto& prm = k.utils.getParam(pID);
                    prm.setModulationDefault();
                }
            };

            knob.comps.emplace_back(std::move(modDial));
            knob.addAndMakeVisible(*knob.comps.back());
        }

        enum { kModDial, kNumComps };
        const auto onResizeMod = isMacro ? [](Knob&){} : [](Knob& k)
        {
            k.layout.place(*k.comps[kModDial], 1, 1, 1, 1, true);
        };

        knob.onResize = [&k = knob, onResizeMod]()
        {
            const auto thicc = k.utils.thicc;
            const auto& layout = k.layout;

            k.knobBounds = layout(0, 0, 3, 2, true).reduced(thicc);
            layout.place(k.label, 0, 2, 3, 1, false);
            k.label.setMaxHeight();

            onResizeMod(k);
            //layout.place(*k.comps[LockButton], 1.5f, 1.5f, 1.5f, 1.5f, true);
        };

        knob.add(Callback([&k = knob, &prm = mainParam]()
        {
                bool shallRepaint = false;

                //const auto lckd = mainParam->isLocked();
                //if (k.locked != lckd)
                //    k.setLocked(lckd);

                const auto vn = prm.getValue();
                const auto md = prm.getModDepth();
                const auto vm = prm.getValMod();
                const auto mb = prm.getModBias();

                auto& vals = k.values;

                if (vals[Value] != vn || vals[ModDepth] != md || vals[ValMod] != vm || vals[ModBias] != mb)
                {
                    vals[Value] = vn;
                    vals[ModDepth] = md;
                    vals[ValMod] = vm;
                    vals[ModBias] = mb;
                    shallRepaint = true;
                }

                if(shallRepaint)
                    k.repaint();
        }, kUpdateParameterCB, cbFPS::k60, true));

        knob.layout.init
        (
            { 1, 1, 1 },
            { 13, 5, 5 }
        );

        const auto angleWidth = PiQuart * 3.f;
        const auto angleRange = angleWidth * 2.f;

        knob.onPaint = [&k = knob, isMacro, angleWidth, angleRange](Graphics& g)
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
                const auto biasAngle = angleRange * vals[ModBias] - angleWidth;

                g.setColour(Colours::c(CID::Bias));
                {
                    Path biasPath;
                    biasPath.addCentredArc
                    (
                        centre.x, centre.y,
                        radiusInner, radiusInner,
                        0.f,
                        0.f, biasAngle,
                        true
                    );
                    Stroke bStroke = strokeType;
                    bStroke.setStrokeThickness(radDif);

                    g.strokePath(biasPath, bStroke);
                }

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

    inline void makeParameter(Knob& knob, PID pID, const String& name)
	{
		makeParameter(knob, &pID, name, 1);
	}
}