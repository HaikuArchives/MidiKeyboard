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


#include <PopUpMenu.h>
#include <View.h>
#include "midi_receiver.h"
#include "midi_engine.h"

const extern uchar MARK_VEL_MAX;
const extern uchar MARK_VEL_MIN;

const extern uint32 CHORD_SETUP;
const extern uint32 CHORD_CLEAR;

class KeyboardView : public BView, public midi::receiver
{
public:
	KeyboardView(BRect frame);
	virtual void Draw(BRect updateRect);

	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual void MouseMoved(BPoint point, uint32 transit, const BMessage* message);

	virtual void KeyDown(const char* bytes, int32 numbytes);
	virtual void KeyUp(const char* bytes, int32 numbytes);

	virtual void MessageReceived(BMessage* message);
private:
	friend class KbWindow;
	friend class KbApplication;
	void draw_key(uchar key);
	void draw_key_no_mark(uchar key);
	void draw_mark(uchar, uchar);

	void f_note_on(uchar key, uchar velocity, bigtime_t when);
	void f_note_off(uchar key, uchar velocity, bigtime_t when);
	int current_key;
	bool key_state[24];
	int keyoctave;
	bool tracking;

	bool chord[12];
	rgb_color mark_color;
	bool m_velocity_marks;
	bool m_num_c;
	BPopUpMenu* create_menu();

	int m_size;
	void set_size(int);
	float size(int);

	uchar get_key(BPoint p);
	BRect get_key_rect(uchar key);

public:
	// receiver
	void note_on(uchar note, uchar velocity);
	void note_off(uchar note, uchar velocity);
	void invalidate_all();
	void no_visible_change();

	// keyboard size
	float Width();
	float Height();
	float KeyWidth();
};
