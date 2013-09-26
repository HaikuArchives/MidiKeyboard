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


#include "KbApplication.h"
#include "KbWindow.h"
#include "KeyboardView.h"
#include "Icons.h"
#include <MidiRoster.h>
#include "KbMidiConsumer.h"
#include "Icons.h"
#include <Alert.h>
#include <Path.h>
#include <FindDirectory.h>
#include <string>
#include <fstream>
#include <Roster.h>
#include <List.h>
#include <Screen.h>
#include <MidiRoster.h>
#include <cstdio>

const uint32 SET_RECEIVE_CHANNEL = 'schR';
const uint32 SET_TRANSMIT_CHANNEL = 'schT';
const uint32 PRODUCER_CONNECT = 'ctPD';
const uint32 CONSUMER_CONNECT = 'ctCS';

KbApplication::KbApplication()
	: BApplication("application/x-vnd.sipkema.midikeyboard")
{
}

void KbApplication::ReadyToRun()
{
	new midi::engine();

	BBitmap* largeIcon = new BBitmap(BRect(0,0,31,31), B_CMAP8);
	BBitmap* miniIcon = new BBitmap(BRect(0,0,15,15), B_CMAP8);
	GetIcons(largeIcon, miniIcon);
	
	BMessage msg;
	BMidiEndpoint* input = BMidiRoster::FindEndpoint(midi_engine->InputID());
	if (input && input->GetProperties(&msg) == B_OK)
	{
		AddIcons(&msg, largeIcon, miniIcon);
		input->SetProperties(&msg);
	}
	input->Register();
	input->Release(); 

	BMidiEndpoint* output = BMidiRoster::FindEndpoint(midi_engine->OutputID());
	if (output && output->GetProperties(&msg) == B_OK)
	{
		AddIcons(&msg, largeIcon, miniIcon);
		output->SetProperties(&msg);
	} 	
	output->Register();
	output->Release(); 

	float pos_x = 100;
	float pos_y = 100;
	float width = 400;
	rgb_color default_mark_color;
	default_mark_color.red = 200;
	default_mark_color.green = 200;
	default_mark_color.blue = 200;
	default_mark_color.alpha = 0;

	BPath path;
	if(find_directory(B_USER_SETTINGS_DIRECTORY, &path) == B_OK)
	{
		string p;
		p = path.Path();
		p += "/midikeyboard_settings";
		ifstream is(p.c_str());
		if(is)
		{
			string version;
			is >> version;
			if(version == "midikeyboard.1.5.1")
			{
				string property;
				while(is >> property)
				{
					if(property == "pos_x")
					{
						float t;
						if(is >> t)
							pos_x = t;
					}
					else if(property == "pos_y")
					{
						float t;
						if(is >> t)
							pos_y = t;
					}
					else if(property == "width")
					{
						float t;
						if(is >> t)
							width = t;
					}
					else if(property == "mark_color")
					{
						char c;
						is >> c;
						if(c == '#')
						{
							char buf[6];
							is.read(buf, 6);
							unsigned int red, green, blue;
							sscanf(buf, "%2x%2x%2x", &red, &green, &blue);
							default_mark_color.red = red;
							default_mark_color.green = green;
							default_mark_color.blue = blue;
						}
					}
				}
			}
		}
	}
	BList teams;
	be_roster->GetAppList("application/x-vnd.sipkema.midikeyboard", &teams);
	int instances = teams.CountItems();
	pos_x += (instances - 1) * 20;
	pos_y += (instances - 1) * 20;
	BScreen screen;
	while(pos_x > (screen.Frame().right - 40))
		pos_x = 40 + (pos_x - (screen.Frame().right - 40));
	while(pos_y > (screen.Frame().bottom - 40))
		pos_y = 40 + (pos_y - (screen.Frame().bottom - 40));
	
	m_keyboard_window = new KbWindow(BRect(pos_x, pos_y, pos_x + width, 100));
	m_keyboard_window->keyboard_view->mark_color = default_mark_color;
	m_keyboard_window->Show();
}

void KbApplication::AboutRequested()
{
	BAlert* about_alert = new BAlert("About MIDI Keyboard"
		, "MIDI Keyboard by Martijn Sipkema"
		"\n1.5.3 GPL, 2002/04/08"
		"\n\nMail bugs or comments to:\nmsipkema@sipkema-digital.com"
		"\n\n© Copyright 2000-2002 Martijn Sipkema"
		"\n\nMIDI Keyboard is free software; you can redistribute it and/or modify " 
    "it under the terms of the GNU General Public License as published by "
    "the Free Software Foundation; either version 2 of the License, or " 
    "(at your option) any later version." 

    "\n\nMIDI Keyboard is distributed in the hope that it will be useful, "
    "but WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
    "GNU General Public License for more details. "

    "\n\nYou should have received a copy of the GNU General Public License "
    "along with MIDI Keyboard; if not, write to the Free Software "
    "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA"
		, "OK");
	about_alert->Go();
}

void KbApplication::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
	case SET_TRANSMIT_CHANNEL:
		{
			int32 channel;
			message->FindInt32("channel", &channel);
			midi_engine->set_transmit_channel((uchar)channel);
			break;
		}
	case SET_RECEIVE_CHANNEL:
		{
			int32 channel;
			message->FindInt32("channel", &channel);
			midi_engine->set_receive_channel((uchar)channel);
			break;
		}
	case PRODUCER_CONNECT:
		{
			int32 id;
			message->FindInt32("id", &id);
			BMidiProducer* producer = BMidiRoster::FindProducer(id);
			BMidiConsumer* input = BMidiRoster::FindConsumer(midi_engine->InputID());
			if(input && producer)
			{
				if(producer->IsConnected(input))
					producer->Disconnect(input);
				else
					producer->Connect(input);
				producer->Release();
				input->Release();
			}
				
			break;
		}
	case CONSUMER_CONNECT:
		{
			int32 id;
			message->FindInt32("id", &id);
			BMidiConsumer* consumer = BMidiRoster::FindConsumer(id);
			BMidiProducer* output = BMidiRoster::FindProducer(midi_engine->OutputID());
			if(output && consumer)
			{
				if(output->IsConnected(consumer))
					output->Disconnect(consumer);
				else
					output->Connect(consumer);
				consumer->Release();
				output->Release();
			}
				
			break;
		}
	default:
		BApplication::MessageReceived(message);
		break;
	}
}

bool KbApplication::QuitRequested()
{
	BApplication::QuitRequested();
	return true;
}

void KbApplication::Quit()
{
	delete midi_engine;
	BApplication::Quit();
}
