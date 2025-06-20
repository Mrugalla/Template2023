#pragma once
#include "Button.h"

namespace gui
{
	struct Prompt :
		public Comp
	{
		Prompt(Utils&);

		void paint(Graphics&) override;

		void resized() override;

		void activate(const PromptData&);

		void deactivate();
	private:
		Label message;
		std::vector<std::unique_ptr<Button>> buttons;
	};
}

/*
organization: make xml file with all possible prompts and give each prompt a unique id
	then make alternative characters for preset browser and tooltip etc
*/