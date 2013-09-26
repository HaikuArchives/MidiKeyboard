// Copyright 1999, Be Incorporated.   All Rights Reserved.
// This file may be used under the terms of the Be Sample Code License.


#ifndef ICONS
#define ICONS

#include <AppFileInfo.h>
#include <Application.h>
#include <Bitmap.h>
#include <File.h>
#include <Roster.h>

extern const char* LARGE_ICON_NAME;
extern const char* MINI_ICON_NAME;
extern const uint32 LARGE_ICON_TYPE;
extern const uint32 MINI_ICON_TYPE;

void AddIcons(BMessage* msg, BBitmap* largeIcon, BBitmap* miniIcon);
void GetIcons(BBitmap* largeIcon, BBitmap* miniIcon);

#endif
