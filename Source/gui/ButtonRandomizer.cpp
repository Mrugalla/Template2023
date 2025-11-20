#include "ButtonRandomizer.h"

namespace gui
{
    ButtonRandomizer::ButtonRandomizer(Utils& u, const String& id) :
        Button(u),
        randomizables(),
        randFuncs(),
        randomizer(*u.audioProcessor.state.props.getUserSettings(), id, false)
    {
        makePaintButton
        (
            *this,
            [](Graphics& g, const Button& b)
            {
                const auto& utils = b.utils;
                const auto thicc = utils.thicc;
                const auto thicc2 = thicc * 2.f;

                const auto hoverPhase = b.callbacks[Button::kHoverAniCB].phase;
                const auto clickPhase = b.callbacks[Button::kClickAniCB].phase;

                const auto lineThiccness = thicc + clickPhase * (thicc2 - thicc);
                const auto margin = thicc2 - lineThiccness - hoverPhase * thicc;
                const auto bounds = maxQuadIn(b.getLocalBounds().toFloat()).reduced(margin);

                const auto iW = bounds.getWidth() / 3.f;
				const auto iH = bounds.getHeight() / 2.f;
				auto iX = bounds.getX();
				
                setCol(g, CID::Interact);
                for (auto i = 0; i < 3; ++i)
                {
                    auto iY = bounds.getY();
                    for (auto j = 0; j < 2; ++j)
                    {
						const auto area = maxQuadIn(BoundsF(iX, iY, iW, iH)).reduced(lineThiccness);
						g.fillEllipse(area);
						iY += iH;
                    }
                    iX += iW;
                }
            },
            makeTooltip()
        );

		onClick = [&](const Mouse& mouse)
        {
            operator()(mouse.mods.isLeftButtonDown(), !mouse.mods.isShiftDown());
        };
    }

	void ButtonRandomizer::add(PID p)
	{
		add(utils.audioProcessor.params[p]);
	}

    void ButtonRandomizer::add(Param* p)
    {
        randomizables.push_back(p);
    }

    void ButtonRandomizer::add(std::vector<Param*>&& ps)
    {
        for (auto p : ps)
            add(p);
    }

    void ButtonRandomizer::add(const std::vector<Param*>& ps)
    {
        for (const auto p : ps)
            add(p);
    }

    void ButtonRandomizer::add(const RandFunc& p)
    {
        randFuncs.push_back(p);
    }

    void ButtonRandomizer::operator()(bool seedUp, bool isAbsolute)
    {
        if(isAbsolute)
            for (auto randomizable : randomizables)
            {
                randomizable->setModDepth(0.f);
            }
        randomizer.updateSeed(seedUp);

        for (auto randomizable : randomizables)
        {
            const auto pID = randomizable->id;
            if (pID != PID::Power
#if PPDIO == PPDIODryWet || PPDIO == PPDIOWetMix
                && pID != PID::GainWet
#endif
                && pID != PID::GainOut
                && pID != PID::Macro)
            {
                const auto& range = randomizable->range;

                float valY, mdY, bsY;
                if (isAbsolute)
                {
                    const auto v = randomizer();
                    const auto vD = range.convertFrom0to1(v);
                    const auto vL = range.snapToLegalValue(vD);
                    valY = range.convertTo0to1(vL);
					const bool up = randomizer() > .5f;
                    if (up)
						mdY = randomizer() * (1.f - valY);
                    else
						mdY = randomizer() * -valY;
                    bsY = randomizer();
                }
                else
                {
                    const auto valNorm = randomizable->getValue();
                    auto valRand = randomizer() * .05f - .025f;
                    valY = juce::jlimit(0.f, 1.f, valNorm + valRand);
                    const auto md = randomizable->getModDepth();
                    const auto bias = randomizable->getModBias();
                    if (md != 0.f)
                    {
                        valRand = randomizer() * .05f - .025f;
                        mdY = juce::jlimit(-1.f, 1.f, md + valRand);
                        valRand = randomizer() * .05f - .025f;
						bsY = juce::jlimit(0.f, 1.f, bias + valRand);
					}
                    else
                    {
                        mdY = md;
						bsY = bias;
                    }
                }
                randomizable->setValueWithGesture(valY);
                if (randomizable->isModulatable())
                {
                    randomizable->setModDepth(mdY);
                    randomizable->setModBias(bsY);
                }
            }
        }
        for (auto& func : randFuncs)
            func(randomizer);

        setTooltip(makeTooltip());
    }

    void ButtonRandomizer::mouseEnter(const Mouse& mouse)
    {
        setTooltip(makeTooltip());
        Button::mouseEnter(mouse);
    }

    String ButtonRandomizer::makeTooltip()
    {
        Random rand;
        static constexpr auto count = 278;
        const auto v = rand.nextInt(count + 1);
        const auto vStr = "#" + String(v) + " ";
        switch (v)
        {
        case 0: return vStr + "Do it!";
        case 1: return vStr + "Don't you dare it!";
        case 2: return vStr + "But... what if it goes wrong???";
        case 3: return vStr + "Nature is random too, so this is basically analog, right?";
        case 4: return vStr + "Life is all about exploration..";
        case 5: return vStr + "What if I don't even exist?";
        case 6: return vStr + "Idk, it's all up to you.";
        case 7: return vStr + "This randomizes the parameter values. Yeah..";
        case 8: return vStr + "Born too early to explore space, born just in time to hit the randomizer.";
        case 9: return vStr + "Imagine someone sitting there writing down all these phrases.";
        case 10: return vStr + "Will this save your snare from sucking ass?";
        case 11: return vStr + "Producer-san >.< d.. don't tickle me there!~";
        case 12: return vStr + "I mean, whatever.";
        case 13: return vStr + "Never commit. Just dream!";
        case 14: return vStr + "I wonder, what will happen if I...";
        case 15: return vStr + "Hit it for the digital warmth!";
        case 16: return vStr + "Don't you love cats? They are so cute :3";
        case 17: return vStr + "We should collab some time, bro.";
        case 18: return vStr + "Did you just hover the button to see what's in here this time?";
        case 19: return vStr + "It's not just a phase!";
        case 20: return vStr + "No time for figuring out parameter values manually, huh?";
        case 21: return vStr + "My cat is meowing at the door because there is a mouse rn lol";
        case 22: return vStr + "Yeeeaaaaahhhh!!!! :)";
        case 23: return vStr + "Ur hacked now >:) no just kidding ^.^";
        case 24: return vStr + "What would you do if your computer could handle 1million phasers?";
        case 25: return vStr + "It's " + (juce::Time::getCurrentTime().getHours() < 10 ? juce::String("0") + static_cast<juce::String>(juce::Time::getCurrentTime().getHours()) : static_cast<juce::String>(juce::Time::getCurrentTime().getHours())) + ":" + (juce::Time::getCurrentTime().getMinutes() < 10 ? juce::String("0") + static_cast<juce::String>(juce::Time::getCurrentTime().getMinutes()) : static_cast<juce::String>(juce::Time::getCurrentTime().getMinutes())) + " o'clock now.";
        case 26: return vStr + "I once was a beat maker, too, but then I took a compressor to the knee.";
        case 27: return vStr + "It's worth a try.";
        case 28: return vStr + "Omg, your music is awesome dude. Keep it up!";
        case 29: return vStr + "I wish there was an anime about music producers.";
        case 30: return vStr + "Days are too short, but I also don't want gravity to get heavier.";
        case 31: return vStr + "Yo, let's order some pizza!";
        case 32: return vStr + "I wanna be the very best, like no one ever was!!";
        case 33: return vStr + "Hm... yeah, that could be cool.";
        case 34: return vStr + "Maybe...";
        case 35: return vStr + "Well.. perhaps.";
        case 36: return vStr + "Here we go again.";
        case 37: return vStr + "What is the certainty of a certainty to mean a certain certainty?";
        case 38: return vStr + "One of my favorite cars is the RX7 so i found it quite funny when Izotope released that plugin.";
        case 39: return vStr + "Do you know Echobode? It's one of my favourite plugins!";
        case 40: return vStr + "Have you ever tried to make eurobeat? It's really hard for some reason.";
        case 41: return vStr + "Wanna lose control?";
        case 42: return vStr + "Do you have any more of dem randomness pls?";
        case 43: return vStr + "How random do you want it to be, sir? Yes.";
        case 44: return vStr + "Programming is not creative. I am a computer.";
        case 45: return vStr + "We should all be more mindful to each other.";
        case 46: return vStr + "Next-Level AI will randomize ur parameters!";
        case 47: return vStr + "All The Award-Winning Audio-Engineers Use This Button!!";
        case 48: return vStr + "The fact that you can't undo it only makes it better.";
        case 49: return vStr + "When things are almost as fast as light, reality bends.";
        case 50: return vStr + "I actually come from the future. Don't tell anyone pls!";
        case 51: return vStr + "You're mad!";
        case 52: return vStr + "Your ad could be here! ;)";
        case 53: return vStr + "What colour-Scheme does your tune sound like?";
        case 54: return vStr + "I wish Dyson Spheres existed already!";
        case 55: return vStr + "This is going to be so cool! OMG";
        case 56: return vStr + "Plants. There should be more of them.";
        case 57: return vStr + "10 Vibrato Mistakes Every Noob Makes: No. 7 Will Make U Give Up On Music!";
        case 58: return vStr + "Yes, I'll add more of these some other time.";
        case 59: return vStr + "The world wasn't ready for No Man's Sky. That's all.";
        case 60: return vStr + "Temposynced tremolos are not sidechain compressors.";
        case 61: return vStr + "I can't even!";
        case 62: return vStr + "Let's drift off into the distance together..";
        case 63: return vStr + "When I started making NEL I wanted to make a tape emulation.";
        case 64: return vStr + "Scientists still trying to figure this one out..";
        case 65: return vStr + "Would you recommend this button to your friends?";
        case 66: return vStr + "This is a really bad feature. Don't use it!";
        case 67: return vStr + "I don't know what to say about this button..";
        case 68: return vStr + "A parallel universe, in which you will use this button now, exists.";
        case 69: return vStr + "This is actually message no. 69, haha";
        case 70: return vStr + "Who needs control anyway?";
        case 71: return vStr + "I have the feeling this time it will turn out really cool!";
        case 72: return vStr + "This turns all parameters up right to 11.";
        case 73: return vStr + "Tranquilize Your Music. Idk why, but it sounds cool.";
        case 74: return vStr + "I'm indecisive...";
        case 75: return vStr + "That's a good idea!";
        case 76: return vStr + "Once upon a time there was a traveller who clicked this button..";
        case 77: return vStr + "10/10 Best Decision!";
        case 78: return vStr + "Beware! Only really skilled audio professionals use this feature.";
        case 79: return vStr + "What would be your melody's name if it was a human being?";
        case 80: return vStr + "What if humanity was just a failed experiment by a higher species?";
        case 81: return vStr + "Enter the black hole to stop time!";
        case 82: return vStr + "Did you remember to water your plants yet?";
        case 83: return vStr + "I'm just a simple button. Nothing special to see here.";
        case 84: return vStr + "You're using this plugin. That makes you a cool person.";
        case 85: return vStr + "Only the greatest DSP technology was used in this parameter randomizer!";
        case 86: return vStr + "I am not fun at parties indeed.";
        case 87: return vStr + "This button makes it worse!";
        case 88: return vStr + "I am not sure what this is going to do.";
        case 89: return vStr + "If your music was a mountain, what shape would it be like?";
        case 90: return vStr + "NEL is the best vibrato plugin in the world. Tell all ur friends!";
        case 91: return vStr + "Do you feel the vibrations?";
        case 92: return vStr + "Defrost or Reheat? You decide!";
        case 93: return vStr + "Don't forget to hydrate yourself, king/queen.";
        case 94: return vStr + "How long does it take to get to the next planet at this speed?";
        case 95: return vStr + "What if there is a huge wall around the whole universe?";
        case 96: return vStr + "Controlled loss of control. So basically drifting! Yeah!";
        case 97: return vStr + "I talk to the wind. My words are all carried away.";
        case 98: return vStr + "Captain, we need to warp now! There is no time.";
        case 99: return vStr + "Where are we now?";
        case 100: return vStr + "Randomize me harder, daddy!~";
        case 101: return vStr + "Drama!";
        case 102: return vStr + "Turn it up! Well, this is not a knob, but you know, it's cool.";
        case 103: return vStr + "You like it dangerous, huh?";
        case 104: return vStr + "QUICK! We are under attack!";
        case 105: return vStr + "Yes, you want this!";
        case 106: return vStr + "The randomizer is better than your presets!";
        case 107: return vStr + "Are you a decide-fan, or a random-enjoyer?";
        case 108: return vStr + "Let's get it started! :D";
        case 109: return vStr + "Do what you have to do...";
        case 110: return vStr + "This is a special strain of random. ;)";
        case 111: return vStr + "Return to the battlefield or get killed.";
        case 112: return vStr + "~<* Easy Peazy Lemon Squeezy *>~";
        case 113: return vStr + "Why does it sound like dubstep?";
        case 114: return vStr + "Excuse me.. Have you seen my sanity?";
        case 115: return vStr + "In case of an emergency, push the button!";
        case 116: return vStr + "Based.";
        case 117: return vStr + "Life is a series of random collisions.";
        case 118: return vStr + "It is actually possible to add too much salt to spaghetti.";
        case 119: return vStr + "You can't go wrong with random, except when you do.";
        case 120: return vStr + "I have not implemented undo yet, but you like to live dangerously :)";
        case 121: return vStr + "404 - Creative message not found. Contact our support pls.";
        case 122: return vStr + "Press jump twice to perform a doub.. oh wait, wrong app.";
        case 123: return vStr + "And now for the ultimate configuration!";
        case 124: return vStr + "Subscribe for more random messages! (Only 42$/mon)";
        case 125: return vStr + "I love you <3";
        case 126: return vStr + "Me? Well...";
        case 127: return vStr + "What happens if I press this?";
        case 128: return vStr + "Artificial Intelligence! Not used here, but it sounds cool.";
        case 129: return vStr + "My internet just broke so why not just write another msg in here, right?";
        case 130: return vStr + "Mood.";
        case 131: return vStr + "I'm only a randomizer, after all...";
        case 132: return vStr + "There is a strong correlation between you and awesomeness.";
        case 133: return vStr + "Yes! Yes! Yes!";
        case 134: return vStr + "Up for a surprise?";
        case 135: return vStr + "This is not a plugin. It is electricity arranged swag-wise.";
        case 136: return vStr + "Chairs do not exist.";
        case 137: return vStr + "There are giant spiders all over my house and I have no idea what to do :<";
        case 138: return vStr + "My cat is lying on my lap purring and she's so cute omg!!";
        case 139: return vStr + "I come here and add more text whenever I procrastinate from fixing bugs.";
        case 140: return vStr + "Meow :3";
        case 141: return vStr + "N.. Nyan? uwu";
        case 142: return vStr + "Let's Go!";
        case 143: return vStr + "Never Gonna Let You Down! Never Gonna Give You Up! sry..";
        case 144: return vStr + "Push It!";
        case 145: return vStr + "Do You Feel The NRG??";
        case 146: return vStr + "We could escape the great filter if we only vibed stronger..";
        case 147: return vStr + "Check The Clock. It's time for randomization.";
        case 148: return vStr + "The first version of NEL was released in 2019.";
        case 149: return vStr + "My first plugin NEL was named after my son, Lionel.";
        case 150: return vStr + "If this plugin breaks, it's because your beat is too fire!";
        case 151: return vStr + "Go for it!";
        case 152: return vStr + "<!> nullptr exception: please contact the developer. <!>";
        case 153: return vStr + "Wild Missingno. appeared!";
        case 154: return vStr + "Do you have cats? Because I love cats. :3";
        case 155: return vStr + "There will be a netflix adaption of this plugin soon.";
        case 156: return vStr + "Studio Gib Ihm!";
        case 157: return vStr + "One Click And It's Perfect!";
        case 158: return vStr + "If you drive a Tesla I want you to remove all of my plugins from your computer.";
        case 159: return vStr + "Remember to drink water, senpai!";
        case 160: return vStr + "Love <3";
        case 161: return vStr + "Your journey has just begun ;)";
        case 162: return vStr + "You will never be the same again...";
        case 163: return vStr + "Black holes are just functors that create this universe inside of this universe.";
        case 164: return vStr + "Feel the heat!";
        case 165: return vStr + "Rightclick on the randomizer to go back to the last seed.";
        case 166: return vStr + "Tbh, that would be crazy.";
        case 167: return vStr + "Your horoscope said you'll make the best beat of all time today.";
        case 168: return vStr + "Do it again! :)";
        case 169: return vStr + "Vibrato is not equal vibrato, dude.";
        case 170: return vStr + "This is going to be all over the place!";
        case 171: return vStr + "Pitch and time... it belongs together.";
        case 172: return vStr + "A rainbow is actually transcendence that never dies.";
        case 173: return vStr + "It is not random. It is destiny!";
        case 174: return vStr + "Joy can enable you to change the world.";
        case 175: return vStr + "This is a very unprofessional plugin. It sucks the 'pro' out of your music.";
        case 176: return vStr + "They tried to achieve perfection, but they didn't hear this beat yet.";
        case 177: return vStr + "Dream.";
        case 178: return vStr + "Music is a mirror of your soul and has the potential to heal.";
        case 179: return vStr + "Lmao, nxt patch is going to be garbage!";
        case 180: return vStr + "Insanity is doing the same thing over and over again.";
        case 181: return vStr + "If you're uninspired just do household-chores. Your brain will try to justify coming back.";
        case 182: return vStr + "You are defining the future standard!";
        case 183: return vStr + "Plugins are a lot like games, but you can't speedrun them.";
        case 184: return vStr + "When applying NEL's audiorate mod to mellow sounds it can make them sorta trumpet-ish.";
        case 185: return vStr + "NEL's oversampling is lightweight and reduces aliasing well, but it also alters the sound a bit.";
        case 186: return vStr + "This message was added 2022_03_15 at 18:10! just in case you wanted to know..";
        case 187: return vStr + "This is message no 187. Ratatatatatat.";
        case 188: return vStr + "Did you ever look at some font like 'This is the font I wanna use for the rest of my life'? Me neither.";
        case 189: return vStr + "Rightclick on the power button and lock its parameter to prevent it from being randomized.";
        case 190: return vStr + "idk...";
        case 191: return vStr + "This is just a placeholder for a real tooltip message.";
        case 192: return vStr + "Let's drift into a new soundscape!";
        case 193: return vStr + "This is the most generic tooltip.";
        case 194: return vStr + "Diffusion can be inharmonic, yet soothing.";
        case 195: return vStr + "I always wanted to make a plugin that has something to do with temperature..";
        case 196: return vStr + "You can't spell 'random' without 'awesome'.";
        case 197: return vStr + "Do you want a 2nd opinion on that?";
        case 198: return vStr + "This is essentially gambling, but without wasting money.";
        case 199: return vStr + "You can lock parameters in order to avoid randomizing them.";
        case 200: return vStr + "Right-click parameters in order to find additional options.";
        case 201: return vStr + "Turn it up to 11 for turbo mode! Oh, oops, wrong parameter. sryy";
        case 202: return vStr + "Bleep bloop. :> I am a computer! Hihi";
        case 203: return vStr + "Mold is basically just tiny mushrooms. You should still not eat it tho.";
        case 204: return vStr + "I wish there was a producer anime, where they'd fight for the best beats.";
        case 205: return vStr + "These tooltip messages have a deep lore.";
        case 206: return vStr + "You can't spell 'awesome' without 'random'. Well, you can, but you shouldn't.";
        case 207: return vStr + "Not in the mood today. Please use a different button! :/";
        case 208: return vStr + "I know you really want to click on this button, but I want you to try something else instead.";
        case 209: return vStr + "Sweet.";
        case 210: return vStr + "OMG I wanna tell you everything about me and my parameter-friends!!!";
        case 211: return vStr + "Take your phone number and add or subtract 1 from it! That's your textdoor neighbor.";
        case 212: return vStr + "Um.. ok?";
        case 213: return vStr + "I need more coffee.";
        case 214: return vStr + "If you feel like supporting me, you can find my Paypal link in my video descriptions. :>";
        case 215: return vStr + "Beware! This button causes quantum entanglement.";
        case 216: return vStr + "Pink is the craziest colour, because it's a mix between the lowest and highest perceivable frequencies of light.";
        case 217: return vStr + "You can't see mirrors. You can only see what mirrors show. What does a mirror look like?";
        case 218: return vStr + "If I was you, I would try to make a beat that sounds like a randomizer.";
        case 219: return vStr + "I am a computer. I am not a human. I am not a human. I am not a human. I am not a human. I am not a";
        case 220: return vStr + "In the future there will be more tooltip messages.";
        case 221: return vStr + "Refresh yourself with a cold shower!";
        case 222: return vStr + "This is the last tooltip message. I promise.";
        case 223: return vStr + "Insanity is the only way to achieve perfection.";
        case 224: return vStr + "Destructive forces cause constructive changes.";
        case 225: return vStr + "Division by Zero is neither undefined nor infinite, but simply insane.";
        case 226: return vStr + "Hold shift as you click this button to randomize sensitively.";
        case 227: return vStr + "Did you know you can hold shift to randomize sensitively?";
        case 228: return vStr + "If you hold shift while clicking on this button, it will randomize sensitively.";
        case 229: return vStr + "Mirror, mirror on the wall, who is the best producer of them all?";
        case 230: return vStr + "Shake things up and see where the music takes you!";
        case 231: return vStr + "Feeling stuck? Let chance be your guide.";
        case 232: return vStr + "Why make choices when the computer can do it for you?";
        case 233: return vStr + "Who needs a muse when you have a randomizer button?";
        case 234: return vStr + "Go ahead, live dangerously and randomize all.";
        case 235: return vStr + "Feeling uninspired? Let randomization be your DJ.";
        case 236: return vStr + "Unlock the secrets of the universe with the press of a button.";
        case 237: return vStr + "Are you ready for a mind-bending musical experience?";
        case 238: return vStr + "The randomize button holds the key to unlocking your true potential.";
        case 239: return vStr + "Unleash the power of the unknown with a press of the button.";
        case 240: return vStr + "Do you dare to uncover the hidden melodies within your music?";
        case 241: return vStr + "The randomize button is your gateway to a new dimension of sound.";
        case 243: return vStr + "I don't get high of drugs. I get high of randomization.";
        case 244: return vStr + "Randomization, like, totally changes your perspective, man.";
        case 245: return vStr + "The randomize button, like, totally expands your mind and stuff.";
        case 246: return vStr + "In the mood for some sonic wilderness?";
        case 247: return vStr + "Because man-made melodies are so last season, press randomize for a fresh natural sound.";
        case 248: return vStr + "Ready to add a little sparkle to your music?";
        case 249: return vStr + "Why play it safe when you can be a little naughty?";
        case 250: return vStr + "Creating music is like catching a dream.";
        case 251: return vStr + "Every note is a door to an alternate reality.";
        case 252: return vStr + "You never know what you will find if you dive deep enough into the sound.";
        case 253: return vStr + "Music is a journey that starts where words end.";
        case 254: return vStr + "Every chord is a story waiting to be told.";
        case 255: return vStr + "I'm sorry for the tooltip messages that were suggested by ChatGPT.";
        case 256: return vStr + "In order to make awesome beats, you have to drive a Miata!";
		case 257: return vStr + "The Miata is the best car for making beats.";
        case 258: return vStr + "You shouldn't ever make beats in a bad mood, because your car could spin out.";
		case 259: return vStr + "The best beats are made in a Miata.";
		case 260: return vStr + "MIATA POWER!!?!";
		case 261: return vStr + "The Mazda Miata has a perfect 50/50 weight distribution.";
		case 262: return vStr + "The Mazda Miatas is an affordable and lightweight sports cars with exceptional balance.";
		case 263: return vStr + "What does your music sound like in a Miata?";
        case 264: return vStr + "Click this to activate celebrity mode!";
        case 265: return vStr + "I don't need drugs to get high. I get high of plugins.";
        case 266: return vStr + "I wouldn't randomize now if I were you.. but I mean, it's your music.";
        case 267: return vStr + "This is not my final form yet.";
        case 268: return vStr + "Making plugins is easy, but it's kinda hard.";
        case 269: return vStr + "Let's do it! :)";
        case 270: return vStr + "hfggddfssdsff, oh sry my cat walked over my keyboard. (tbh i wrote that myself)";
        case 271: return vStr + "You can use my plugin, Absorbiere, as a beatsynced oscilloscope.";
        case 272: return vStr + "Delete all your social media accounts now!";
        case 273: return vStr + "Fuck Au5!";
        case 274: return vStr + "I don't remember specific things, I just stumble into patterns.";
        case 275: return vStr + "Release the Epstein files!";
        case 276: return vStr + "If you think infinity is crazy, imagine what it would be like if everything had an end.";
        case 277: return vStr + "Legalize LSD!";
        case 278: return vStr + "If you sometimes see pixels with your bare eyes, that's visual snow syndrom.";
        default: "Are you sure?";
        }
        return "You are not supposed to read this message!";
    }
}