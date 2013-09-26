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


#include "StatusView.h"
#include "KbApplication.h"
#include "KbWindow.h"
#include <Message.h>
#include <Region.h>
#include <Window.h>
#include <MenuItem.h>
#include "midi_engine.h"
#include "MidiProducerMenu.h"
#include "MidiConsumerMenu.h"
#include <cstdio>

const uint32 UPDATE = 'updt';

const bigtime_t FADE_TIME = 500000;

StatusView::StatusView(BRect frame)
	: BView(frame, "status", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM
		, B_WILL_DRAW)
		, m_blink(false), m_last_note(-1)
		, m_note_number(false)
		, m_update_runner(0)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	SetFont(be_fixed_font);
}

void StatusView::AttachedToWindow()
{
	m_update_runner = new BMessageRunner(this, new BMessage(UPDATE)
		, 50000, -1);
	if(m_update_runner->InitCheck() != B_OK)
		throw;
}
void StatusView::Draw(BRect updateRect)
{
	// border
	BRect r = Bounds();
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT));
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT));
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));
	StrokeLine(BPoint(r.right, r.bottom), BPoint(r.right, r.top + 1));
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT));
	r.InsetBy(1, 1);
	r.left--;
	StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.top));
	StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));
	r.left++; r.top++;

	BRect b = Bounds();
	b.InsetBy(4, 4);
	b.bottom += 1;
	b.left -= 1;
	b.right = b.left + b.Height();

	BRegion clip(Bounds());
	clip.Exclude(b.InsetByCopy(-1, -1));
	ConstrainClippingRegion(&clip);
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(r);
	ConstrainClippingRegion(0);

	if(m_blink)
	{
		bigtime_t t = system_time() - midi_engine->last_data();
		uchar red, green, blue;
		float tf = t / float(FADE_TIME);
		if(tf > 1) tf = 1;
		red = uchar(164 - 100 * tf);
		green = uchar(80 - 80 * tf);
		blue = uchar(255 - 127 * tf);
		SetHighColor(red, green, blue);
		FillRect(b);
		b.InsetBy(-1, -1);


		red = uchar(230 - 100 * tf);
		green = uchar(230 - 100 * tf);
		blue = uchar(230 - 100 * tf);
		SetHighColor(red, green, blue);
		
		StrokeLine(BPoint(b.left + 1, b.bottom - 2), BPoint(b.left + 1, b.top + 1));
		StrokeLine(BPoint(b.left + 1, b.top + 1), BPoint(b.right - 2, b.top + 1));

		SetHighColor(0, 0, 0);
		
		StrokeRect(b);
	}
	else
	{
		SetHighColor(64, 0, 128);
		FillRect(b);
		b.InsetBy(-1, -1);
		SetHighColor(130, 130, 130);
		StrokeLine(BPoint(b.left + 1, b.bottom - 2), BPoint(b.left + 1, b.top + 1));
		StrokeLine(BPoint(b.left + 1, b.top + 1), BPoint(b.right - 2, b.top + 1));
		SetHighColor(0, 0, 0);
		StrokeRect(b);
	}

	if(m_last_note >= 0)
	{
		BString ss;
		if(!m_note_number)
		{
			switch(m_last_note % 12)
			{
			case 0:
				ss << "C";
				break;
			case 1:
				ss << "C#";
				break;
			case 2:
				ss << "D";
				break;
			case 3:
				ss << "D#";
				break;
			case 4:
				ss << "E";
				break;
			case 5:
				ss << "F";
				break;
			case 6:
				ss << "F#";
				break;
			case 7:
				ss << "G";
				break;
			case 8:
				ss << "G#";
				break;
			case 9:
				ss << "A";
				break;
			case 10:
				ss << "A#";
				break;
			case 11:
				ss << "B";
				break;
			}
			ss << m_last_note / 12 - 1;
		}
		else
		{
			ss << m_last_note;
		}
		SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		SetHighColor(0, 0, 0);
		MovePenTo(b.right + 8, Bounds().bottom - 2);
		DrawString(ss.String());
	}
}

void StatusView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case UPDATE:
		update();
		break;
	default:
		BView::MessageReceived(message);
		break;
	}
}

void StatusView::MouseDown(BPoint where)
{
	int32 buttons;
	Window()->CurrentMessage()->FindInt32("buttons", &buttons);
	switch(buttons)
	{
	case B_PRIMARY_MOUSE_BUTTON:
		{
			BPoint p(Bounds().left, Bounds().bottom + 2);
			ConvertToScreen(&p);
			BPopUpMenu* menu = create_menu();
			menu->SetAsyncAutoDestruct(true);
			if(Window()->Feel() == B_NORMAL_WINDOW_FEEL)
				menu->Go(p, true, true, ConvertToScreen(Bounds()), true);
			else
				menu->Go(p, true, false, true);
			break;
		}
	case B_SECONDARY_MOUSE_BUTTON:
		{
			if(m_note_number)
				m_note_number = false;
			else
				m_note_number = true;
			Invalidate();
			break;
		}
	}
}


StatusView::~StatusView()
{
	delete m_update_runner;
}

void StatusView::note_on(uchar note, uchar velocity)
{
	update();
}

void StatusView::note_off(uchar note, uchar velocity)
{
	// does nothing
}

void StatusView::update()
{

	if(Window()->Lock())
	{
		int last_note = -1;
		bigtime_t time = system_time();
		if(time - midi_engine->last_note_time() < 1000000)
			last_note = midi_engine->last_note();
		bool blink = false;
		if(time - midi_engine->last_data() < FADE_TIME)
			blink = true;
	
		if(last_note != m_last_note || blink != m_blink)
		{
			m_last_note = last_note;
			m_blink = blink;
			Invalidate();
		}
		else if(blink) // only invalidate led-rectangle
		{
			BRect b = Bounds();
			b.InsetBy(4, 4);
			b.bottom += 1;
			b.left -= 1;
			b.right = b.left + b.Height();
			b.InsetBy(-1, -1);
			Invalidate(b);
		}
		Window()->Unlock();
	}
}

void StatusView::no_visible_change()
{
	update();
}

void StatusView::invalidate_all()
{
	update();
}


BPopUpMenu* StatusView::create_menu()
{
	BPopUpMenu* menu = new BPopUpMenu("menu", false, false);

	BMessage* msg;
	BMenuItem* item;

	item = new BMenuItem("Panic", new BMessage(ALL_NOTES_OFF));
	item->SetShortcut('P', B_COMMAND_KEY);
	item->SetTarget(Window(), Window());
	menu->AddItem(item);	
	menu->AddSeparatorItem();
	// transmit channel menu
	BMenu* chan_transmit = new BMenu("Transmit Channel");

	for(int i = 0; i < 16; ++i)
	{
		char cnum[3];
		sprintf(cnum, "%i", i + 1);
		msg = new BMessage(SET_TRANSMIT_CHANNEL);
		msg->AddInt32("channel", i);
		item = new BMenuItem(cnum, msg);
		item->SetTarget(be_app_messenger);
		chan_transmit->AddItem(item);
	}
//	chan_transmit->SetRadioMode(true);
//	(chan_transmit->ItemAt(0))->SetMarked(true);
	(chan_transmit->ItemAt(midi_engine->transmit_channel()))->SetMarked(true);
	menu->AddItem(chan_transmit);

	// receive channel menu
	BMenu* chan_receive = new BMenu("Receive Channel");

	for(int i = 0; i < 16; ++i)
	{
		char itemt[3];
		sprintf(itemt, "%i", i + 1);
		msg = new BMessage(SET_RECEIVE_CHANNEL);
		msg->AddInt32("channel", i);
		item = new BMenuItem(itemt, msg);
		item->SetTarget(be_app_messenger);
		chan_receive->AddItem(item);
	}
//	chan_receive->SetRadioMode(true);
//	(chan_receive->ItemAt(0))->SetMarked(true);
	(chan_receive->ItemAt(midi_engine->receive_channel()))->SetMarked(true);
	menu->AddItem(chan_receive);

	menu->AddSeparatorItem();

	// input menu
		MidiProducerMenu* input_menu = new MidiProducerMenu("Input", midi_engine->InputID()
		, be_app_messenger, new BMessage(PRODUCER_CONNECT));
	menu->AddItem(input_menu);

	// output menu
	MidiConsumerMenu* output_menu = new MidiConsumerMenu("Output", midi_engine->OutputID()
		, be_app_messenger, new BMessage(CONSUMER_CONNECT));
	menu->AddItem(output_menu);
	menu->AddSeparatorItem();
	BMenuItem* about_item = new BMenuItem("About MIDI Keyboard…", new BMessage(B_ABOUT_REQUESTED));
	about_item->SetTarget(be_app_messenger);
	menu->AddItem(about_item);

	menu->AddSeparatorItem();
	BMenuItem* quit_item = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED));
	quit_item->SetTarget(be_app_messenger);
	quit_item->SetShortcut('Q', B_COMMAND_KEY);
	menu->AddItem(quit_item);

	return menu;
}