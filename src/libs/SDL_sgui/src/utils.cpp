
#include <SDL/sgui/sdl-headers.h>
#include <SDL/SDL_sgui.h>

namespace Gui
{
	void Utils::message(const ustring &title, const ustring &msg)
	{
		Label *label = new Label("", msg);
		Button *button = new Button("", "  ok  ");
		label->setAlignment(Label::CENTER);
		button->addListener(actionCloseWindow());

		Widget &center = Spacer_(true)/ *label / Spacer_(true) / (Spacer_(false) | *button | Spacer_(false)) / Spacer_(true);

		putenv((char*)"SDL_VIDEO_CENTERED=1");
		Window wnd("", std::max(320, center.getOptimalWidth() + 32), center.getOptimalHeight() + 16, Window::MOVEABLE);
		wnd.addChild(center);
		wnd.setTitle(title);
		wnd();
	}

	ustring Utils::input(const ustring &title, const ustring &msg)
	{
		Label *label = new Label("", msg);
		Button *button = new Button("", "  ok  ");
		LineInput *input = new LineInput("");
		label->setAlignment(Label::CENTER);
		button->addListener(actionCloseWindow());
		input->addListener(actionCloseWindow());

		Widget &center = Spacer_(true) / *label / Spacer_(true) / *input / Spacer_(true) / (Spacer_(false) | *button | Spacer_(false)) / Spacer_(true);

		putenv((char*)"SDL_VIDEO_CENTERED=1");
		Window wnd("", std::max<int>(320, center.getOptimalWidth() + 32), center.getOptimalHeight() + 16, Window::MOVEABLE);
		wnd.addChild(center);
		wnd.setTitle(title);
		wnd();
		return input->getText();
	}

	bool Utils::ask(const ustring &title, const ustring &msg)
	{
		bool bOk = false;

		Label *label = new Label("", msg);
		Button *ok = new Button("", "    ok    ");
		Button *cancel = new Button("", "  cancel  ");
		label->setAlignment(Label::CENTER);
		ok->addListener(actionCloseWindow());
		ok->addListener(actionSetBool(bOk));
		cancel->addListener(actionCloseWindow());

		Widget &center = Spacer_(true) / *label / Spacer_(true) / (Spacer_(false) | *ok | Spacer_(false) | *cancel | Spacer_(false)) / Spacer_(true);

		putenv((char*)"SDL_VIDEO_CENTERED=1");
		Window wnd("", std::max(320, center.getOptimalWidth() + 32), center.getOptimalHeight() + 16, Window::MOVEABLE);
		wnd.addChild(center);
		wnd.setTitle(title);
		wnd();
		return bOk;
	}

	namespace Actions
	{
		class CloseWindow : public Receiver
		{
		protected:
			virtual void proc(Widget *)
			{
				Widget::emitEvent(Widget::EVENT_CLOSE);
			}
		};

		class SetBool : public Receiver
		{
		public:
			SetBool(bool &b) : b(b)	{}
		protected:
			virtual void proc(Widget *)
			{
				b = true;
			}

		private:
			bool &b;
		};
	}

	Receiver *Utils::actionCloseWindow()
	{
		return new Actions::CloseWindow;
	}

	Receiver *Utils::actionSetBool(bool &b)
	{
		return new Actions::SetBool(b);
	}
}
