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


#include "midi_engine.h"
#include "KbMidiConsumer.h"

midi::engine* midi_engine;

namespace midi {

const int engine::M_RECVS_WRITE_ACQUIRE = 10;

engine::engine()
	: m_receive_channel(0), m_transmit_channel(0)
		, m_last_data(0), m_last_note_time(0), m_kbstate_changed(false)
{
	midi_engine = this;

	if( (m_lock_sem = create_sem(1, 0)) < B_OK )
		throw 1;
	if( (m_recvs_sem = create_sem(M_RECVS_WRITE_ACQUIRE, 0)) < B_OK )
		throw 1;
	if( (m_inv_sem = create_sem(1, 0)) < B_OK )
		throw 1;
	if( (m_state_lock = create_sem(1, 0)) < B_OK )
		throw 1;

	if( (m_inv_thread = spawn_thread(inv_thread_func, "ViewInvalidater"
		, B_NORMAL_PRIORITY, 0)) < 0)
			throw 1;
	acquire_sem(m_inv_sem);
	if(resume_thread(m_inv_thread) != B_NO_ERROR)
		throw 1;

	m_input = new KbMidiConsumer("MIDI Keyboard input");
	m_output = new BMidiLocalProducer("MIDI Keyboard output");
}

engine::~engine()
{
	m_output->Release();
	m_input->Release();

	send_data(m_inv_thread, 0, 0, 0);
	suspend_thread(m_inv_thread);
	resume_thread(m_inv_thread);
	wait_for_thread(m_inv_thread, 0);

	delete_sem(m_inv_sem);
	delete_sem(m_lock_sem);
	delete_sem(m_recvs_sem);
	delete_sem(m_state_lock);
}

// midiconsumer
void engine::note_on(uchar channel, uchar note, uchar velocity, bigtime_t time)
{
	if(channel == receive_channel())
		set_note(note, velocity, (time)?time:system_time());
}

void engine::note_off(uchar channel, uchar note, uchar velocity)
{
	if(channel == receive_channel())
		set_note(note, 0);
}

void engine::control_change(uchar channel, uchar controlnumber
	, uchar controlvalue)
{
	if(channel != receive_channel())
		return;

	if(controlnumber == 64)
		set_sustain( (controlvalue > 63)?true:false );
}

void engine::data(bigtime_t time)
{
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		m_last_data = time;
		// midi event has a time 0, so we give it a time
		if(!m_last_data)
			m_last_data = system_time();

		release_sem(m_lock_sem);
	} 
}

// state inquiry
uchar engine::vs_note(uchar note)
{
	uchar v = 0;
	if(acquire_sem(midi_engine->m_state_lock) == B_NO_ERROR)
	{
		v = m_view_side_state.get_note(note);
		release_sem(m_state_lock);
	}
	return v;
}

note_s engine::note(uchar note)
{
	note_s n; n.note = note; n.velocity = 0;
	if( (note < 128) && acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		n.velocity = m_kbstate.get_note(note);
		release_sem(m_lock_sem);
	}
	return n;
}

bool engine::sustain()
{
	bool s = false;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		s = m_kbstate.is_sustain();
		release_sem(m_lock_sem);
	} 
	return s;
}

bigtime_t engine::last_data()
{
	bigtime_t t = 0;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		t = m_last_data;
		release_sem(m_lock_sem);
	} 
	return t;
}

uchar engine::last_note()
{
	uchar t = 0;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		t = m_last_note;
		release_sem(m_lock_sem);
	} 
	return t;
}

bigtime_t engine::last_note_time()
{
	bigtime_t t = 0;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		t = m_last_note_time;
		release_sem(m_lock_sem);
	} 
	return t;
}

// state mod
void engine::set_note(uchar note, uchar velocity, bigtime_t time)
{
	if(note > 127) return;

	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		note_s n; n.note = note; n.velocity = velocity;
		if(n.velocity) // note_on
		{
			// last note and last note time
			if(time) m_last_note_time = time;
			m_last_note = note;

			m_kbstate.note_on(note, velocity);
		}
		else // note_off
			m_kbstate.note_off(note);

		if(!m_kbstate_changed)
		{
			m_kbstate_changed = true;
			release_sem_etc(m_inv_sem, 1, B_DO_NOT_RESCHEDULE);
		}
		
		release_sem(m_lock_sem);
	}
}

void engine::set_sustain(bool sustain)
{
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		m_kbstate.sustain(sustain);

		if(!m_kbstate_changed)
		{
			m_kbstate_changed = true;
			release_sem_etc(m_inv_sem, 1, B_DO_NOT_RESCHEDULE);
		}

		release_sem(m_lock_sem);
	}
}

void engine::set_all_notes_off()
{
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		m_kbstate.all_notes_off();

		if(!m_kbstate_changed)
		{
			m_kbstate_changed = true;
			release_sem_etc(m_inv_sem, 1, B_DO_NOT_RESCHEDULE);
		}

		release_sem(m_lock_sem);
	}
}

// receivers
void engine::add_receiver(receiver* r)
{
	if(acquire_sem_etc(m_recvs_sem
		, M_RECVS_WRITE_ACQUIRE, 0, 0) == B_NO_ERROR)
	{
		m_receivers.push_back(r);
		release_sem_etc(m_recvs_sem
			, M_RECVS_WRITE_ACQUIRE, 0);
	}
}

void engine::clear_receivers()
{
	if(acquire_sem_etc(m_recvs_sem
		, M_RECVS_WRITE_ACQUIRE, 0, 0) == B_NO_ERROR)
	{
		m_receivers.clear();
		release_sem_etc(m_recvs_sem
			, M_RECVS_WRITE_ACQUIRE, 0);
	}
}


// channels
uchar engine::receive_channel()
{
	uchar channel = 0;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		channel = m_receive_channel;
		release_sem(m_lock_sem);
	}
	return channel;
}

uchar engine::transmit_channel()
{
	uchar channel = 0;
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		channel = m_transmit_channel;
		release_sem(m_lock_sem);
	}
	return channel;
}

void engine::set_receive_channel(uchar channel)
{
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		m_receive_channel = channel;
		release_sem(m_lock_sem);
	}
}

void engine::set_transmit_channel(uchar channel)
{
	if(acquire_sem(m_lock_sem) == B_NO_ERROR)
	{
		m_transmit_channel = channel;
		release_sem(m_lock_sem);
	}
}

// local midi input
void engine::local_note_on(uchar channel, uchar note, uchar velocity
	, bigtime_t time)
{
	m_output->SprayNoteOn(channel, note, velocity, time);
	set_note(note, velocity, time);
}

void engine::local_note_off(uchar channel, uchar note, uchar velocity
	, bigtime_t time)
{
	m_output->SprayNoteOff(channel, note, velocity, time);
	set_note(note, 0);
}

void engine::local_control_change(uchar channel, uchar controlnumber
		,uchar controlvalue, bigtime_t time)
{
	if(controlnumber == 64)
		set_sustain((controlvalue < 64)?false:true);
	if(controlnumber == 123)
		set_all_notes_off();
	m_output->SprayControlChange(channel, controlnumber
			, controlvalue, time);
}

// input/output
int32 engine::InputID()
{
	return m_input->ID();
}

int32 engine::OutputID()
{
	return m_output->ID();
}

int32 engine::inv_thread_func(void* data)
{
	for(;;)
	{
		if( has_data(find_thread(0)))
		{
			exit_thread(0);
		}
		if(acquire_sem(midi_engine->m_inv_sem) == B_NO_ERROR)
		{
			keyboard_state ks;
			// get the current state from the engine
			// and clear the engine's state changed flag.
			if(acquire_sem(midi_engine->m_lock_sem) == B_NO_ERROR)
			{
				ks = midi_engine->m_kbstate;
				midi_engine->m_kbstate_changed = false;
				release_sem(midi_engine->m_lock_sem);
			}
			// compare our state with the new state and notify
			// receivers of changes.
			key_list kl;
			if(acquire_sem(midi_engine->m_state_lock) == B_NO_ERROR)
			{
				kl = midi_engine->m_view_side_state.compare(ks);
				// set our state to the new state.
				midi_engine->m_view_side_state = ks;
				release_sem(midi_engine->m_state_lock);
			}
			if(acquire_sem(midi_engine->m_recvs_sem) == B_NO_ERROR)
			{
				for(list<receiver*>::iterator i = midi_engine->m_receivers.begin();
				i != midi_engine->m_receivers.end(); ++i)
				{
					if(kl.size() < 10)
					{
						if(kl.size())
						{
							for(key_list::iterator it = kl.begin(); it != kl.end(); ++it)
							{
								(*i)->note_on(it->note, it->velocity);
							}
						}
						else (*i)->no_visible_change();
					}
					else
					{
						(*i)->invalidate_all();
					}
				}
				release_sem(midi_engine->m_recvs_sem);
			}
		}
	}	
}

} // namespace midi