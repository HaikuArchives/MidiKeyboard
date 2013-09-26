/*
    Copyright 2000-2002, Martijn J.W. Sipkema	

    This file is part of midikeyboard. 

    midikeyboard is free software; you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation; either version 2 of the License, or 
    (at your option) any later version. 

    midikeyboard is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details. 

    You should have received a copy of the GNU General Public License 
    along with midikeyboard; if not, write to the Free Software 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    In addition, as a special exception, Martijn J.W. Sipkema gives permission
    to link the code of this program with the IconMenuItem-1.0.0 library and
    Icons.h/Icons.cc that came with midikeyboard, and distribute linked
    combinations including the two.  You must obey the GNU General Public
    License in all respects for all of the code used other than the
    IconMenuItem-1.0.0 library and Icons.h/Icons.cc that came with
    midikeyboard.  If you modify this file, you may extend this exception to
    your version of the ile, but you are not obligated to do so.  If you do
    not wish to do so, delete this exception statement from your version.
*/


#include "KbWindow.h"
#include "KeyboardView.h"
#include "StatusView.h"
#include <MenuItem.h>
#include <Path.h>
#include <string>
#include <fstream>
#include <FindDirectory.h>
#include "midi_engine.h"
#include "KbApplication.h"
#include <Roster.h>

const uint32 ALL_NOTES_OFF = 'aoff';
const uint32 RESIZE = 'rsze';

KbWindow::KbWindow(BRect frame)
	: BWindow(frame, 0
		, B_TITLED_WINDOW
		, 0
		)
{
	be_roster->StartWatching(BMessenger(0, this));
	set_title();

	AddShortcut('P', B_COMMAND_KEY, new BMessage(ALL_NOTES_OFF));	
	keyboard_view = new KeyboardView(BRect(0, 0, 0, 0));
	AddChild(keyboard_view);

	ResizeTo(Bounds().Width()
		, keyboard_view->Frame().bottom + B_H_SCROLL_BAR_HEIGHT);

	BRect r(0, 0, 50, B_H_SCROLL_BAR_HEIGHT - 1);

	scrollbar = new BScrollBar(r, "scrollbar", keyboard_view
		, 0, keyboard_view->Bounds().Width() - Bounds().Width()
		, B_HORIZONTAL);
	scrollbar->SetResizingMode(B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	AddChild(scrollbar);

	status_view = new StatusView(r);
	status_view->SetResizingMode(B_FOLLOW_LEFT | B_FOLLOW_TOP);
	AddChild(status_view);

	midi_engine->add_receiver(status_view);
	midi_engine->add_receiver(keyboard_view);

	Resize();

	keyboard_view->MakeFocus(true);
}

bool KbWindow::QuitRequested()
{
	be_roster->StopWatching(BMessenger(0, this));

	Unlock();
	midi_engine->clear_receivers();
	Lock();

	// save window position
	float pos_x = Frame().left;
	float pos_y = Frame().top;
	float width = Frame().Width();

	BPath path;
	if(find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK)
	{
		string p;
		p = path.Path();
		p += "/midikeyboard_settings";
		ofstream os(p.c_str());
		if(os)
		{
			os << "midikeyboard.1.5.1\n";
			os << "pos_x\t" << pos_x << endl;
			os << "pos_y\t" << pos_y << endl;
			os << "width\t" << width << endl;
			os.setf(ios::hex, ios::basefield);
			os << "mark_color\t" << '#';
			os.fill('0');
			os.width(2);
			os	<< (int)keyboard_view->mark_color.red;
			os.width(2);
			os	<< (int)keyboard_view->mark_color.green;
			os.width(2);
			os	<< (int)keyboard_view->mark_color.blue << endl;
		}
	}
///////////////
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void KbWindow::FrameResized(float width, float height)
{
	scrollbar->SetRange(0, keyboard_view->Width() - Bounds().Width());
	scrollbar->SetProportion(Bounds().Width() / keyboard_view->Width());
	scrollbar->SetSteps(keyboard_view->KeyWidth(), Bounds().Width() + 1);
}

void KbWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case B_MOUSE_WHEEL_CHANGED:
		{
			float dy;
			message->FindFloat("be:wheel_delta_y", &dy);
			scrollbar->SetValue(scrollbar->Value() - (dy * keyboard_view->KeyWidth() * 7));
			break;
		}
	case ALL_NOTES_OFF:
		{
			int64 when;
			message->FindInt64("when", &when);
			midi_engine->local_control_change(midi_engine->transmit_channel()
				, 123, 0, when);
			break;
		}
	case RESIZE:
		Resize();
		break;
	case B_SOME_APP_LAUNCHED: // fall through
	case B_SOME_APP_QUIT:
		set_title();
		break;
	default:
		BWindow::MessageReceived(message);
	}
}

void KbWindow::Resize()
{
	// is scrollbar visible?
	bool visible_scrollbar = false;
	if(Frame().Height() > keyboard_view->Frame().Height())
		visible_scrollbar = true;
	// set alignment and limits
	SetSizeLimits(keyboard_view->KeyWidth() * 14 - 1
		, keyboard_view->KeyWidth() * 75 - 1, keyboard_view->Height()
		, keyboard_view->Height() + B_H_SCROLL_BAR_HEIGHT);

	SetWindowAlignment(B_PIXEL_ALIGNMENT, 0, 0, 0, 0, 0, 0
		, B_H_SCROLL_BAR_HEIGHT
		, (int(keyboard_view->Height()) + int(B_H_SCROLL_BAR_HEIGHT) + 1)
			% int(B_H_SCROLL_BAR_HEIGHT));
	// resize window
	float w = Bounds().Width();
	if(w < (keyboard_view->KeyWidth() * 14 - 1))
		w = (keyboard_view->KeyWidth() * 14 - 1);
	else if(w > (keyboard_view->KeyWidth() * 75 - 1))
		w = (keyboard_view->KeyWidth() * 75 - 1);
	ResizeTo(w, keyboard_view->Height());
	if(visible_scrollbar)
		ResizeBy(0, B_H_SCROLL_BAR_HEIGHT);
	// sync	
	Sync();
	// reposition and resize views
	keyboard_view->ResizeTo(Bounds().Width(), keyboard_view->Height());
	status_view->MoveTo(BPoint(0, keyboard_view->Frame().bottom + 1));
	scrollbar->MoveTo(BPoint(status_view->Frame().right + 1
		, keyboard_view->Frame().bottom + 1));
	scrollbar->ResizeTo(1 + Frame().Width() - (status_view->Frame().Width() + 1)
		, B_H_SCROLL_BAR_HEIGHT);
	// invalidate views
	status_view->Invalidate();
	keyboard_view->Invalidate();
	/////////////////////////////////
}

void KbWindow::Zoom(BPoint origin, float width, float height)
{
	// don't want the height to change
	BWindow::Zoom(origin, width, Frame().Height());
}

void KbWindow::set_title()
{
	BString title = "MIDI Keyboard";
	BList teams;
	be_roster->GetAppList("application/x-vnd.sipkema.midikeyboard", &teams);
	if(teams.CountItems() > 1)
	{
		title 
			<< " >" << midi_engine->InputID()
			<< " <" << midi_engine->OutputID();
	}
	
	SetTitle(title.String());
}