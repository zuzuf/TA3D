
#include <SDL/sgui/sdl-headers.h>
#include <SDL_image.h>
#include <SDL/SDL_sgui.h>

#include <iostream>
using namespace std;
using namespace Gui;

class QuitListener : public Receiver
{
protected:
	virtual void proc(Widget *)
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	}
};

class ActionListener : public Receiver
{
protected:
	virtual void proc(const std::wstring &name)
	{
		if (name == L"entry5")
			TABWIDGET(tab)->setCurrentTab(1);
		cout << toUtf8(name) << endl;
	}
};

#undef main

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	Window wnd("window", 800, 600, Window::RESIZEABLE);
	wnd.setTitle("Hello World!");
//	wnd.setLayout(new HBoxLayout);
	wnd.setLayout(new UnmanagedLayout);
	wnd.setResizeable(true);

	wnd.addChild(TabWidget_("tab"));
	TABWIDGET(tab)->setLayout(new UnmanagedLayout);
	TABWIDGET(tab)->resize(800,400);
//	TABWIDGET(tab)->setFrameLess(true);

	wnd.setMenuBar(MenuBar_("menubar"));
	MENUBAR(menubar)->addMenu(Menu_("M1", "Menu1"));
	MENUBAR(menubar)->addMenu(Menu_("M2", "Menu2"));

	MENU(M1)->addEntry("entry0", "hello");
	MENU(M1)->addEntry("", "");
	MENU(M1)->addEntry("entry1", "exit");
	MENU(M2)->addEntry("entry2", Menu_("M3", "Menu3"));
	MENU(M2)->addEntry("entry3", "entrée normale");
	MENU(M3)->addEntry("entry4", "sous entrée");
	MENU(M3)->addEntry("entry5", "sous entrée 2");

	LINK(WIDGET(entry1), new QuitListener);
	LINK(WIDGET(entry0), new ActionListener);
	LINK(WIDGET(entry5), new ActionListener);

	TABWIDGET(tab)->addTab("Tab 1", Spacer_(true)
						   / (Spacer_(false) | Button_("ok") | Spacer_(false))
						   / Spacer_(true));

	TABWIDGET(tab)->addTab("Tab 2", TextEdit_("text"));

	TABWIDGET(tab)->addTab("Tab 3", Label_("label", "Ceci est un morceau de texte\nc'est tout à fait digeste\nc'est encodé en UTF-8\n\npourtant c'est affiché par des fonctions unicode \\o/"));

	TABWIDGET(tab)->addTab("Tab 4", ScrollArea_("scroll"));

	BUTTON(ok)->setCaption("   oké   ");
	BUTTON(ok)->addListener(new QuitListener);

	TABWIDGET(tab)->addTab("Tab 5", SpinBox_("spin")
								  / Spacer_(true));

	SCROLLAREA(scroll)->setCentralWidget(Group_("group", "group", Button_("", "   o<   ")
										 / Picture_("", IMG_Load("image.png"))
										 / Picture_("", IMG_Load("image.png"))
										 / Picture_("", IMG_Load("image.png"))
										 / Picture_("", IMG_Load("image.png"))
										 / Picture_("", IMG_Load("image.png"))));


	SPINBOX(spin)->setPrecision(2);
	SPINBOX(spin)->setMinimum(1);
	SPINBOX(spin)->setStep(0.1);
	SPINBOX(spin)->setMaximum(10);
	SPINBOX(spin)->setValue(5);
	TABWIDGET(tab)->addTab("Tab 6", LineInput_("line"));

	wnd();
	return 0;
}
