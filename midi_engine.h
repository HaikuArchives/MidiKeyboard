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


#ifndef MIDI_ENGINE_H
#define MIDI_ENGINE_H

#include <list>
#include <OS.h>
#include <MidiProducer.h>
#include <MidiConsumer.h>
#include "note_s.h"
#include "midi_receiver.h"
#include "keyboard_state.h"

namespace midi {

class engine
{
private:
	
	list<receiver*> m_receivers;
	sem_id m_lock_sem;
	sem_id m_recvs_sem;
	sem_id m_inv_sem;
	sem_id m_state_lock;
	thread_id m_inv_thread;
	static int32 inv_thread_func(void*);
	static const int M_RECVS_WRITE_ACQUIRE;
//	uchar m_notes_on[128];
//	uchar m_notes_sustained[128];
//	bool m_sustain;
	uchar m_receive_channel;
	uchar m_transmit_channel;
	bigtime_t m_last_data;
	uchar m_last_note;
	bigtime_t m_last_note_time;
	keyboard_state m_view_side_state;
	keyboard_state m_kbstate;
	bool m_kbstate_changed;
	BMidiLocalProducer* m_output;
	BMidiLocalConsumer* m_input;
public:
	engine();
	~engine();

	// local midi input
	void local_note_on(uchar channel, uchar note, uchar velocity
		, bigtime_t time);
	void local_note_off(uchar channel, uchar note, uchar velocity
		, bigtime_t time);
	void local_control_change(uchar channel, uchar controlnumber
		,uchar controlvalue, bigtime_t time);

	// state inquiry
	uchar vs_note(uchar note);
	note_s note(uchar note);
	bool sustain();
	bigtime_t last_data();
	uchar last_note();
	bigtime_t last_note_time();

	// receivers
	// don't call after consumer is created
	void add_receiver(receiver*);
	void clear_receivers();

	// channels
	uchar receive_channel();
	uchar transmit_channel();
	void set_receive_channel(uchar channel);
	void set_transmit_channel(uchar channel);

	// input/output
	int32 InputID();
	int32 OutputID();

//////////////////////////////////////////////////
private:
	// state mod
	void set_note(uchar note, uchar velocity, bigtime_t time = 0);
	void set_sustain(bool);
	void set_all_notes_off();
public:
	// called by midiconsumer
	void note_on(uchar channel, uchar note, uchar velocity, bigtime_t time);
	void note_off(uchar channel, uchar note, uchar velocity);
	void control_change(uchar channel, uchar controlnumber
		,uchar controlvalue);
	void data(bigtime_t time);
};

} // namespace midi

extern midi::engine* midi_engine;

#endif // MIDI_ENGINE_H