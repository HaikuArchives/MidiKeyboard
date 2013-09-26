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


#include "keyboard_state.h"
#include <algorithm>
#include <functional>

key::key(unsigned char n, unsigned char v
	, bool s// = false
	)
		: note(n), velocity(v), sustaining(s)
{
}

bool key::operator== (const key& c) const
{
	return ( (c.note == note) && (c.velocity == velocity)
		&& (c.sustaining == c.sustaining) );
}

class note_is : public unary_function<key, bool>
{
public:
	note_is(unsigned char n) {
		note = n;
	}
	unsigned char note;
	bool operator() (const key& k) const {
		return (k.note == note);
	};
};

keyboard_state::keyboard_state()
	: m_sustain(false)
{
}

// set state
void keyboard_state::note_on(unsigned char note, unsigned char velocity)
{
	key_list::iterator i = find_if(m_keys.begin(), m_keys.end(), note_is(note));
	if(i != m_keys.end())
	{
		i->velocity = velocity;
		i->sustaining = false;
	}
	else
		m_keys.push_back(key(note, velocity, false));
	
}

void keyboard_state::note_off(unsigned char note)
{
	key_list::iterator i = find_if(m_keys.begin(), m_keys.end(), note_is(note));
	if(i != m_keys.end())
	{
		if(m_sustain)
			i->sustaining = true;
		else
			m_keys.erase(i);
	}
}

void keyboard_state::sustain(bool sustain)
{
	if(m_sustain != sustain)
	{
		m_sustain = sustain;
		if(m_sustain)
			return;
		key_list::iterator i = m_keys.begin();
		for(;;)
		{
			key_list::iterator t = m_keys.end();
			if(i->sustaining)
				t = i;
			++i;
			if(t != m_keys.end()) m_keys.erase(t);
			if(i == m_keys.end())
				break;			
		}
	}
}

void keyboard_state::all_notes_off()
{
	sustain(false);
	m_keys.clear();			
}


// ask state
unsigned char keyboard_state::get_note(unsigned char note)
{
	key_list::iterator i = find_if(m_keys.begin(), m_keys.end(), note_is(note));
	if(i != m_keys.end())
		return i->velocity;
	else return 0;
}

bool keyboard_state::is_sustain() const
{
	return m_sustain;
}

// compare
key_list keyboard_state::compare(keyboard_state& c)
{
	key_list r;
	// find keys that we don't have.
	for(key_list::iterator i = c.m_keys.begin();
		i != c.m_keys.end(); ++i)
	{
		key_list::iterator ii = find_if(m_keys.begin()
			, m_keys.end(), note_is(i->note));
		if(ii == m_keys.end()) // we don't have it
			r.push_back(*i);
	}
	// check if the keys we do have are equal.
	for(key_list::iterator i = m_keys.begin();
		i != m_keys.end(); ++i)
	{
		key_list::iterator ii = find_if(c.m_keys.begin()
			, c.m_keys.end(), note_is(i->note));
		if(ii == c.m_keys.end()) // doens't exist 
			r.push_back(key(i->note, 0));
		else if( !(*ii == *i) )// not equal
		// the above stupid !(==) is for powerpc
			r.push_back(*ii);
	}
	return r;
	
}