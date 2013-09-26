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


#include "KeyboardView.h"
#include "KbApplication.h"
#include <Window.h>
#include <Region.h>
#include <Message.h>
#include "KbWindow.h"
#include <MenuItem.h>
#include <cstdio>

enum {
	WHITE_KEY_WIDTH = 0
	, BLACK_KEY_WIDTH
	, WHITE_KEY_HEIGHT
	, BLACK_KEY_HEIGHT
	, MARK_HEIGHT
	, MARK_OFFSET
};

const float S_WHITE_KEY_WIDTH = 15;
const float S_BLACK_KEY_WIDTH = 10;
const float S_WHITE_KEY_HEIGHT = 100;
const float S_BLACK_KEY_HEIGHT = 63;
const float S_MARK_HEIGHT = 9;
const float S_MARK_OFFSET = 4;
const float S_SIZES[] = { 
	S_WHITE_KEY_WIDTH
	, S_BLACK_KEY_WIDTH
	, S_WHITE_KEY_HEIGHT
	, S_BLACK_KEY_HEIGHT
	, S_MARK_HEIGHT
	, S_MARK_OFFSET
};

const float M_WHITE_KEY_WIDTH = 17;
const float M_BLACK_KEY_WIDTH = 10;
const float M_WHITE_KEY_HEIGHT = 110;
const float M_BLACK_KEY_HEIGHT = 72;
const float M_MARK_HEIGHT = 11;
const float M_MARK_OFFSET = 6;
const float M_SIZES[] = { 
	M_WHITE_KEY_WIDTH
	, M_BLACK_KEY_WIDTH
	, M_WHITE_KEY_HEIGHT
	, M_BLACK_KEY_HEIGHT
	, M_MARK_HEIGHT
	, M_MARK_OFFSET
};


const float L_WHITE_KEY_WIDTH = 21;
const float L_BLACK_KEY_WIDTH = 12;
const float L_WHITE_KEY_HEIGHT = 125;
const float L_BLACK_KEY_HEIGHT = 83;
const float L_MARK_HEIGHT = 12;
const float L_MARK_OFFSET = 8;
const float L_SIZES[] = { 
	L_WHITE_KEY_WIDTH
	, L_BLACK_KEY_WIDTH
	, L_WHITE_KEY_HEIGHT
	, L_BLACK_KEY_HEIGHT
	, L_MARK_HEIGHT
	, L_MARK_OFFSET
};

const float* SIZES[] = { S_SIZES, M_SIZES, L_SIZES };

enum {
	SMALL = 0
	, MEDIUM
	, LARGE
};

const extern uchar MARK_VEL_MAX = 120;
const extern uchar MARK_VEL_MIN = 15;

const uint32 CHORD_SETUP = 'kvCS';
const uint32 CHORD_CLEAR = 'kvCC';
const uint32 SET_MARK_COLOR = 'kcSM';
const uint32 SET_MARK_ON_OFF = 'kvMS';
const uint32 SET_NUM_C = 'kvNC';
const uint32 SIZE = 'kvSZ';
const uint32 FLOAT = 'wflt';

bool is_white(uchar);

KeyboardView::KeyboardView(BRect frame)
	: BView(frame, "keyboard", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP
		, B_WILL_DRAW), current_key(-1), keyoctave(4), tracking(false)
		, m_velocity_marks(true), m_num_c(false), m_size(SMALL)
{
	for(int i = 0; i < 24; i++)
		key_state[i] = false;
	for(int i = 0; i < 12; i++)
		chord[i] = false;
	SetViewColor(B_TRANSPARENT_COLOR);
}


void KeyboardView::Draw(BRect updateRect)
{
	Window()->BeginViewTransaction();
// draw top and bottom line
	SetHighColor(0, 0, 0);
	StrokeLine(BPoint(0, 0), BPoint(Width(), 0));
	StrokeLine(BPoint(0, size(WHITE_KEY_HEIGHT) - 1), BPoint(Width(), size(WHITE_KEY_HEIGHT) - 1));

// calculate keys to be drawn and call draw_key() for each
	BRegion clip;
	GetClippingRegion(&clip);
	int l_wk = int(clip.Frame().left) / int(size(WHITE_KEY_WIDTH));
	int r_wk = int(clip.Frame().right) / int(size(WHITE_KEY_WIDTH));
	int l_oct = l_wk / 7;
	int r_oct = r_wk / 7;
	uchar l_key = l_oct * 12;
	uchar r_key = r_oct * 12;
	l_key += ((l_wk % 7) < 3)?((l_wk % 7) * 2):(((l_wk % 7) * 2) - 1);
	if(l_key)
		l_key--;
	r_key += ((r_wk % 7) < 3)?((r_wk % 7) * 2):(((r_wk % 7) * 2) - 1);
	r_key++;
	if(l_key > 127)
		l_key = 127;
	if(r_key > 127)
		r_key = 127;
	for(int i = l_key; i <= r_key; i++)
	{
		if(m_velocity_marks)
			draw_key(i);
		else
			draw_key_no_mark(i);
	}
	Window()->EndViewTransaction();
}


void KeyboardView::draw_mark(uchar key, uchar vel = 127)
{
	BRect r = get_key_rect(key);
	r.InsetBy(2, 2);
	//////////////////
	if(!is_white(key))
		r.OffsetBy(0, - size(MARK_OFFSET));
	r.top = r.bottom - (size(MARK_HEIGHT) - 1);
	if(vel > MARK_VEL_MAX) vel = MARK_VEL_MAX;
	if(vel < MARK_VEL_MIN) vel = MARK_VEL_MIN;
	float v = (float)vel / MARK_VEL_MAX;
	r.top -= r.Height() * v;
	mark_color.alpha = uint8(255 * v);
	SetDrawingMode(B_OP_ALPHA);
	SetHighColor(mark_color);
	FillRect(r);

	if(is_white(key))
		SetHighColor(tint_color(HighColor(), B_DARKEN_1_TINT));
	else
		SetHighColor(tint_color(HighColor(), B_LIGHTEN_1_TINT));
	SetDrawingMode(B_OP_COPY);
	StrokeRect(r);
}

void KeyboardView::draw_key_no_mark(uchar key)
{
	if(key > 127) return;
	uchar v = midi_engine->vs_note(key);
	if(is_white(key))
	{
		BRect r = get_key_rect(key);
		if(v) SetHighColor(236, 236, 236);
		else SetHighColor(255, 255, 255);
		BRect r1(r);
		r1.InsetBy(1, 1);
		r1.bottom -= 1;
		BRect r2(r1);
		r1.top = size(BLACK_KEY_HEIGHT);
		FillRect(r1);
		r2.bottom = r1.top + 1;
		if(key)
			r2.left = get_key_rect(key - 1).right + 1;
		else r2.left = r1.left;
		if(key < 127)
			r2.right = get_key_rect(key + 1).left - 1;
		else
			r2.right = r2.right;
		FillRect(r2);

		SetHighColor(0, 0, 0);
		StrokeLine(BPoint(r.right, 0), BPoint(r.right, size(WHITE_KEY_HEIGHT) - 1));
		if(v) SetHighColor(236, 236, 236);
		else SetHighColor(170, 170, 170);

		StrokeLine(BPoint(r1.left, r1.bottom + 1), BPoint(r1.right, r1.bottom + 1));

		if(!v) SetHighColor(200, 200, 200);
		else SetHighColor(100, 100, 100);
		if(!key)
		{
			if(v) SetHighColor(0, 0, 0);
			else SetHighColor(96, 96, 96);
		}
		StrokeLine(
			BPoint(r.left, ( ((key % 12) == 0) || ((key % 12) == 5) ) ? 1 : size(BLACK_KEY_HEIGHT))
			, BPoint(r.left, size(WHITE_KEY_HEIGHT) - 2));
			


		if( m_num_c && !(key % 12))
		{
			if(!v) SetLowColor(255, 255, 255);
			else SetLowColor(236, 236, 236);
			SetHighColor(96, 96, 96);

			BFont f = *be_bold_font;
			switch(m_size)
			{
			case SMALL:
				f.SetSize(9);
				break;
			case MEDIUM:
				f.SetSize(11);
				break;
			case LARGE:
				f.SetSize(12);
				break;
			}
			SetFont(&f);
			int c;
			c = key / 12;
			c -= 1;
			BString s;
			s << c;
			r1 = get_key_rect(key);
			float w = f.StringWidth(s.String()) ;
			MovePenTo(r1.left + (r1.Width() - w) / 2 + .5, r1.bottom + 1 - 5);
			DrawString(s.String());
		}
	}
	else
	{
		BRect r = get_key_rect(key);
		if(!v) SetHighColor(64, 64, 64);
		else SetHighColor(0, 0, 0);
		FillRect(r);
		r.InsetBy(1, 1);
		float pb = r.bottom;
		////////////////////////
		if(v) r.bottom -= size(MARK_OFFSET) / 2;
		else r.bottom -= size(MARK_OFFSET);


		if(!v) SetHighColor(216, 216, 216);
		else SetHighColor(140, 140, 140);

		StrokeLine(BPoint(r.left, r.top), BPoint(r.left, r.bottom));
		StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));
		/////////////////
		if(!v) SetHighColor(180, 180, 180);
		else SetHighColor(100, 100, 100);


		StrokeLine(BPoint(r.left, r.bottom + 1), BPoint(r.left, pb));
		StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));

		SetHighColor(255, 255, 255);
		StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.bottom));
	}
}

void KeyboardView::draw_key(uchar key)
{
	if(key > 127) return;
	uchar v = midi_engine->vs_note(key);
	if(is_white(key))
	{
		BRect r = get_key_rect(key);
		SetHighColor(255, 255, 255);
		BRect r1(r);
		r1.InsetBy(1, 1);
		BRect r2(r1);
		r1.top = size(BLACK_KEY_HEIGHT);
		FillRect(r1);
		r2.bottom = r1.top + 1;
		if(key)
			r2.left = get_key_rect(key - 1).right + 1;
		else r2.left = r1.left;
		if(key < 127)
			r2.right = get_key_rect(key + 1).left - 1;
		else
			r2.right = r2.right;
		FillRect(r2);

		SetHighColor(0, 0, 0);
		StrokeLine(BPoint(r.right, 0), BPoint(r.right, size(WHITE_KEY_HEIGHT) - 1));
		SetHighColor(96, 96, 96);
		StrokeLine(
			BPoint(r.left, ( ((key % 12) == 0) || ((key % 12) == 5) ) ? 1 : size(BLACK_KEY_HEIGHT))
			, BPoint(r.left, size(WHITE_KEY_HEIGHT) - 2));

		if(v) draw_mark(key, v);

		if(!v && m_num_c && !(key % 12))
		{
			SetLowColor(255, 255, 255);
			SetHighColor(140, 140, 140);

			BFont f = *be_bold_font;
			switch(m_size)
			{
			case SMALL:
				f.SetSize(9);
				break;
			case MEDIUM:
				f.SetSize(11);
				break;
			case LARGE:
				f.SetSize(12);
				break;
			}
			SetFont(&f);
			int c;
			c = key / 12;
			c -= 1;
			BString s;
			s << c;
			float w = f.StringWidth(s.String());
			r1 = get_key_rect(key);
			MovePenTo(r1.left + (r1.Width() - w) / 2 + .5, r1.bottom + 1 - 5);
			DrawString(s.String());
		}
	}
	else
	{
		BRect r = get_key_rect(key);
		SetHighColor(0, 0, 0);
		FillRect(r);
		r.InsetBy(1, 1);
		float pb = r.bottom;
		////////////////////////
		r.bottom -= size(MARK_OFFSET);
		SetHighColor(140, 140, 140);
		StrokeLine(BPoint(r.left, r.top), BPoint(r.left, r.bottom));
		StrokeLine(BPoint(r.left, r.bottom), BPoint(r.right, r.bottom));
		/////////////////
		StrokeLine(BPoint(r.left, r.bottom + 1), BPoint(r.left, pb));
		StrokeLine(BPoint(r.left, r.top), BPoint(r.right, r.top));

		SetHighColor(255, 255, 255);
		StrokeLine(BPoint(r.left, r.bottom), BPoint(r.left, r.bottom));
		if(v) draw_mark(key, v);
	}
}


bool is_white(uchar key)
{
	key %= 12;
	switch(key)
	{
	case 1:
	case 3:
	case 6:
	case 8:
	case 10:
		return false;
	default:
		return true;
	}
}

uchar KeyboardView::get_key(BPoint p)
{
	int wk = int(p.x) / int(size(WHITE_KEY_WIDTH));
	int k = ((wk / 7) * 12);
	k += ((wk % 7) < 3)?((wk % 7) * 2):(((wk % 7) * 2) - 1);

	if( (k > 0) && get_key_rect(k - 1).Contains(p) )
		k--;
	else if( (k < 120) && get_key_rect(k + 1).Contains(p) )
		k++;

	if(p.y < 0 && k > 0)
	{
		int kl, kr;
		kl = (is_white(k - 1))?(k - 2):(k - 1);
		kr = (is_white( k + 1))?(k + 2):(k + 1);
		k = ( (p.x - get_key_rect(kl).right) < (p.x - get_key_rect(kr).left) )?kl:kr;
	}

	return k;

}

BRect KeyboardView::get_key_rect(uchar key)
{
	BRect r;
	r.top = 0;
	r.left = (key / 12) * (size(WHITE_KEY_WIDTH) * 7);
	if(is_white(key))
	{
		r.bottom = size(WHITE_KEY_HEIGHT) - 1;
		r.left += ((key % 12) < 4)?(((key % 12) / 2) * size(WHITE_KEY_WIDTH)):((((key % 12) + 1) / 2) * size(WHITE_KEY_WIDTH));
		r.right = r.left + size(WHITE_KEY_WIDTH) - 1;		
	}
	else
	{
		r.bottom = size(BLACK_KEY_HEIGHT) - 1;
		r.left += (((key - 1) % 12) < 4)?((((key % 12) - 1) / 2) * size(WHITE_KEY_WIDTH)):(((((key % 12) - 1) + 1) / 2) * size(WHITE_KEY_WIDTH));
		r.left += size(WHITE_KEY_WIDTH) - (size(BLACK_KEY_WIDTH) / 2);
		r.right = r.left + size(BLACK_KEY_WIDTH) - 1;		
	}
	return r;
}



void KeyboardView::MouseDown(BPoint p)
{
	uchar velocity = 127;
	BMessage* msg = Window()->CurrentMessage();
	int32 buttons;
	msg->FindInt32("buttons", &buttons);

	int64 w;
	msg->FindInt64("when", &w);
	bigtime_t when = w;
	
	switch(buttons)
	{
	case B_PRIMARY_MOUSE_BUTTON:
		{
			tracking = true; // fall through!
			velocity = 100;
		}
	case B_TERTIARY_MOUSE_BUTTON:
		{
			SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
			uchar key = get_key(p);
			current_key = key;
			float v;
			if(is_white(key))
				v = (p.y - size(BLACK_KEY_HEIGHT)) / (size(WHITE_KEY_HEIGHT) - size(BLACK_KEY_HEIGHT)); 
			else
				v = p.y / size(BLACK_KEY_HEIGHT);
			if(v < .5 && buttons == B_TERTIARY_MOUSE_BUTTON) velocity = 63;
			////////////////////////////
			f_note_on(key, velocity, when);
			break;
		}
	case B_SECONDARY_MOUSE_BUTTON:
		{
			BRect r;
			r.left = p.x -10; r.right = p.x + 10;
			r.top = p.y - 10; r.bottom = p.y + 10;
			BPopUpMenu* menu = create_menu();
			menu->SetAsyncAutoDestruct(true);
			if(Window()->Feel() == B_NORMAL_WINDOW_FEEL)
				menu->Go(ConvertToScreen(p), true, true, ConvertToScreen(r), true);
			else
				menu->Go(ConvertToScreen(p), true, false, true);
			break;
		}
	}
}

void KeyboardView::MouseUp(BPoint p)
{
	if(tracking)
		tracking = false;
	if(current_key < 0) return;

	BMessage* msg = Window()->CurrentMessage();
	int64 w;
	msg->FindInt64("when", &w);
	bigtime_t when = w;

	f_note_off(current_key, 0, when);
	current_key = -1;
}

void KeyboardView::MouseMoved(BPoint point, uint32 transit, const BMessage* message)
{
	if(!tracking) return;

	BMessage* msg = Window()->CurrentMessage();
	int64 w;
	msg->FindInt64("when", &w);
	bigtime_t when = w;

	uchar key;
	if (point.x < 0)
		key = 0;
	else if(point.x > Width())
		key = 127;
	else key = get_key(point);
	if(uchar(current_key) != key)
	{
		f_note_off(current_key, 0, when);
		current_key = key;
		f_note_on(key, 100, when);
	}
}


void KeyboardView::KeyDown(const char* bytes, int32 numbytes)
{
	BMessage* msg = Window()->CurrentMessage();

	int64 w;
	msg->FindInt64("when", &w);
	bigtime_t when = w;


	int32 c = 0;
	uchar key = 0;
	status_t err = msg->FindInt32("be:key_repeat", &c);
	// key repeat, not interested in
	if(!err)
	{
		BView::KeyDown(bytes, numbytes);
		return;
	}
	//	if(!c) // initial keypress
	// this did not work like expected when more than on key was pressed
	// at a time, so i'll keep a state, but it still works for the arrow keys
	msg->FindInt32("key", &c);
	switch(c)
	{
	case 0x4c: // C
		key = 0;
		break;
	case 0x3d: // C#
		key = 1;
		break;
	case 0x4d: // D
		key = 2;
		break;
	case 0x3e: // D#
		key = 3;
		break;
	case 0x4e: // E
		key = 4;
		break;
	case 0x4f: // F
		key = 5;
		break;
	case 0x40: // F#
		key = 6;
		break;
	case 0x50: // G
		key = 7;
		break;
	case 0x41: // G#
		key = 8;
		break;
	case 0x51: // A
		key = 9;
		break;
	case 0x42: // A#
		key = 10;
		break;
	case 0x52: // B
		key = 11;
		break;
	case 0x27: // C2
		key = 12;
		break;
	case 0x13: // C#2
		key = 13;
		break;
	case 0x28: // D2
		key = 14;
		break;
	case 0x14: // D#2
		key = 15;
		break;
	case 0x29: // E2
		key = 16;
		break;
	case 0x2a: // F2
		key = 17;
		break;
	case 0x16: // F#2
		key = 18;
		break;
	case 0x2b: // G2
		key = 19;
		break;
	case 0x17: // G#2
		key = 20;
		break;
	case 0x2c: // A2
		key = 21;
		break;
	case 0x18: // A#2
		key = 22;
		break;
	case 0x2d: // B2
		key = 23;
		break;
	case 0x62:
		for(int i = 0; i < 24; i++)
		{
			if(key_state[i])
			{
				key_state[i] = false;
				if( (keyoctave * 12 + i) < 128 )
					f_note_off(keyoctave * 12 + i, 0, when);
			}
		}
		keyoctave--;
		if(keyoctave < 0) keyoctave = 0;
		return;
	case 0x57:
		for(int i = 0; i < 24; i++)
		{
			if(key_state[i])
			{
				key_state[i] = false;
				if( (keyoctave * 12 + i) < 128 )
					f_note_off(keyoctave * 12 + i, 0, when);
			}
		}
		keyoctave++;
		if(keyoctave > 9) keyoctave = 9;
		return;
	case 0x5e: // SUSTAIN
		midi_engine->local_control_change(midi_engine->transmit_channel()
			, 64, 127, when);
		return;
	

	default:
		BView::KeyDown(bytes, numbytes);
		return;
	}
	if(key_state[key]) return;
	key_state[key] = true;
	if( (keyoctave * 12 + key) > 127 ) return;

	f_note_on(keyoctave * 12 + key, 100, when);

}

void KeyboardView::KeyUp(const char* bytes, int32 numbytes)
{
	BMessage* msg = Window()->CurrentMessage();

	int64 w;
	msg->FindInt64("when", &w);
	bigtime_t when = w;

	int32 c = 0;
	uchar key = 0;
	msg->FindInt32("key", &c);
	switch(c)
	{
	case 0x4c: // C
		key = 0;
		break;
	case 0x3d: // C#
		key = 1;
		break;
	case 0x4d: // D
		key = 2;
		break;
	case 0x3e: // D#
		key = 3;
		break;
	case 0x4e: // E
		key = 4;
		break;
	case 0x4f: // F
		key = 5;
		break;
	case 0x40: // F#
		key = 6;
		break;
	case 0x50: // G
		key = 7;
		break;
	case 0x41: // G#
		key = 8;
		break;
	case 0x51: // A
		key = 9;
		break;
	case 0x42: // A#
		key = 10;
		break;
	case 0x52: // B
		key = 11;
		break;
	case 0x27: // C2
		key = 12;
		break;
	case 0x13: // C#2
		key = 13;
		break;
	case 0x28: // D2
		key = 14;
		break;
	case 0x14: // D#2
		key = 15;
		break;
	case 0x29: // E2
		key = 16;
		break;
	case 0x2a: // F2
		key = 17;
		break;
	case 0x16: // F#2
		key = 18;
		break;
	case 0x2b: // G2
		key = 19;
		break;
	case 0x17: // G#2
		key = 20;
		break;
	case 0x2c: // A2
		key = 21;
		break;
	case 0x18: // A#2
		key = 22;
		break;
	case 0x2d: // B2
		key = 23;
		break;
	case 0x5e: // SUSTAIN
		midi_engine->local_control_change(midi_engine->transmit_channel()
			, 64, 0, when);
		return;

	default:
		BView::KeyDown(bytes, numbytes);
		return;
	}
	if(!key_state[key]) return; // for octave change whilst holding key
	key_state[key] = false;
	if( (keyoctave * 12 + key) > 127 ) return;


	f_note_off(keyoctave * 12 + key, 0, when);
}

void KeyboardView::f_note_on(uchar key, uchar velocity, bigtime_t when)
{
	midi_engine->local_note_on(midi_engine->transmit_channel(), key, velocity, when);
	for(int i = 0; i < 12; ++i)
		if(chord[i]) midi_engine->local_note_on(midi_engine->transmit_channel()
			, key + i + 1, velocity, when);
}

void KeyboardView::f_note_off(uchar key, uchar velocity, bigtime_t when)
{
	midi_engine->local_note_off(midi_engine->transmit_channel(), key, velocity, when);
	for(int i = 0; i < 12; ++i)
		if(chord[i]) midi_engine->local_note_off(midi_engine->transmit_channel()
			, key + i + 1, velocity, when);
}

void KeyboardView::note_on(uchar note, uchar velocity)
{
	if(Window()->Lock())
	{
		Invalidate(get_key_rect(note));
		Window()->Unlock();
	}
}

void KeyboardView::note_off(uchar note, uchar velocity)
{
	if(Window()->Lock())
	{
		Invalidate(get_key_rect(note));
		Window()->Unlock();
	}
}

void KeyboardView::invalidate_all()
{
	if(Window()->Lock())
	{
		Invalidate();
		Window()->Unlock();
	}
}

void KeyboardView::no_visible_change()
{
	// does nothing
}	
	

void KeyboardView::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case CHORD_SETUP:
		{
			int32 key;
			message->FindInt32("key", &key);
			key--;
			if(chord[key])
				chord[key] = false;
			else
				chord[key] = true;
			break;
		}
	case CHORD_CLEAR:
		{
			for(int i = 0; i < 12; ++ i)
				chord[i] = false;
			break;
		}
	case B_PASTE:
		{
			const rgb_color* c = 0;
			ssize_t l = 0;
			if(!message->FindData("RGBColor", B_RGB_COLOR_TYPE, (const void**)&c
				, &l))
			{
				if(l == sizeof(mark_color))
					mark_color = *c;
			}
			if(m_velocity_marks)
				Invalidate();
			break;
		}
	case SET_MARK_ON_OFF:
		{
			if(m_velocity_marks)
				m_velocity_marks = false;
			else
				m_velocity_marks = true;
			Invalidate();
			break;
		}
	case SET_NUM_C:
		{
			if(m_num_c)
				m_num_c = false;
			else
				m_num_c = true;
			Invalidate();
			break;
		}
	case SIZE:
		{
			int32 size;
			message->FindInt32("size", &size);
			set_size(size);
			break;
		}
	case FLOAT:
		{
			if(Window()->Feel() == B_FLOATING_ALL_WINDOW_FEEL)
			{
				Window()->SetFeel(B_NORMAL_WINDOW_FEEL);
				Window()->SetLook(B_TITLED_WINDOW_LOOK);
				Window()->SetFlags(0);
			}
			else
			{
				Window()->SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
				Window()->SetLook(B_MODAL_WINDOW_LOOK);
				Window()->SetFlags(B_AVOID_FOCUS);
			}				
			break;
		}
	default:
		BView::MessageReceived(message);
		break;
	}
}

BPopUpMenu* KeyboardView::create_menu()
{
	BPopUpMenu* pop_up_menu = new BPopUpMenu(0, false);
	pop_up_menu->SetRadioMode(false);
	BMessage* msg;
	BMenuItem* item;
	// mark on/off
	item = new BMenuItem("Marks", new BMessage(SET_MARK_ON_OFF));
	item->SetTarget(this);
	if(m_velocity_marks)
		item->SetMarked(true);
	pop_up_menu->AddItem(item);
	/////
	item = new BMenuItem("Octave Numbers", new BMessage(SET_NUM_C));
	item->SetTarget(this);
	if(m_num_c)
		item->SetMarked(true);
	pop_up_menu->AddItem(item);
	///////
	pop_up_menu->AddSeparatorItem();
	// size menu
	BMenu* size_menu = new BMenu("Size");
	msg = new BMessage(SIZE);
	msg->AddInt32("size", SMALL);
	item = new BMenuItem("Small", msg);
	size_menu->AddItem(item);
	msg = new BMessage(SIZE);
	msg->AddInt32("size", MEDIUM);
	item = new BMenuItem("Medium", msg);
	size_menu->AddItem(item);
	msg = new BMessage(SIZE);
	msg->AddInt32("size", LARGE);
	item = new BMenuItem("Large", msg);
	size_menu->AddItem(item);
	size_menu->ItemAt(m_size)->SetMarked(true);
	size_menu->SetTargetForItems(this);
	pop_up_menu->AddItem(size_menu);
	//
	item = new BMenuItem("Float", new BMessage(FLOAT));
	item->SetTarget(this);
	pop_up_menu->AddItem(item);
	if(Window()->Feel() == B_FLOATING_ALL_WINDOW_FEEL)
		item->SetMarked(true);
	////
	pop_up_menu->AddSeparatorItem();
	// chord menu
	BMenu* chord_menu = new BMenu("Chord");
	chord_menu->SetRadioMode(false);
	for(int i = 1; i <= 12; ++i)
	{
		char itemt[4];
		itemt[0] = '+';
		sprintf(itemt + 1, "%i", i);
		msg = new BMessage(CHORD_SETUP);
		msg->AddInt32("key", i);
		item = new BMenuItem(itemt, msg);
		if(chord[i - 1])
			item->SetMarked(true);
		chord_menu->AddItem(item);
	}
	chord_menu->AddSeparatorItem();
	item = new BMenuItem("Clear", new BMessage(CHORD_CLEAR));
	chord_menu->AddItem(item);
	chord_menu->SetTargetForItems(this);
	pop_up_menu->AddItem(chord_menu);

	return pop_up_menu;
}

void KeyboardView::set_size(int s)
{
	if(m_size == s)
		return;
	m_size = s;
	BMessage* msg = new BMessage(RESIZE);
	Window()->PostMessage(msg);
	delete msg;
}

float KeyboardView::Width()
{
	return (size(WHITE_KEY_WIDTH) * ((10 * 7) + 5)) - 1;
}

float KeyboardView::Height()
{
	return size(WHITE_KEY_HEIGHT) - 1;
}

float KeyboardView::KeyWidth()
{
	return size(WHITE_KEY_WIDTH);
}

float KeyboardView::size(int s)
{
	return SIZES[m_size][s];
}
