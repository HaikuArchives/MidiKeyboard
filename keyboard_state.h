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


#include <list>

using namespace std;

class key
{
public:
	unsigned char note;
	unsigned char velocity;
	bool sustaining;
	key();
//	key(const key&);
//	using default
	key(unsigned char note, unsigned char velocity, bool sustaining = false);
	bool operator== (const key& c) const;
};

typedef list<key> key_list;

class keyboard_state
{
private:
	key_list m_keys;
	bool m_sustain;
public:
	keyboard_state();
//	keyboard_state(const keyboard_state&);
//	~keyboard_state();
// 	using defaults

	// set state
	void note_on(unsigned char note, unsigned char velocity);
	void note_off(unsigned char note);
	void sustain(bool sustain);
	void all_notes_off();

	// ask state
	unsigned char get_note(unsigned char note);
	bool is_sustain() const;

	// compare
	key_list compare(keyboard_state&);
};
