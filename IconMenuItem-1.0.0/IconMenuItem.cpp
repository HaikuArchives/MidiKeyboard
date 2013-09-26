#include "IconMenuItem.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
IconMenuItem::IconMenuItem(const char* label,BMessage *message,char shortcut,uint32 modifiers,BBitmap *bitmap)
	:BMenuItem(label,message,shortcut,modifiers)
{
	fBitmap = bitmap;
	fHeightDelta = 0;
}

/***********************************************************
 * Destructor.
 ***********************************************************/
IconMenuItem::~IconMenuItem()
{
	delete fBitmap;
}

/***********************************************************
 * Draw menu icon.
 ***********************************************************/
void
IconMenuItem::DrawContent()
{
		
	if(fBitmap != NULL)
	{
		
		BPoint drawPoint(ContentLocation());
		// center text and icon.	
		Menu()->SetDrawingMode(B_OP_OVER);
		Menu()->SetLowColor(B_TRANSPARENT_32_BIT);
		if(this->IsEnabled() == false)
		{
			Menu()->SetDrawingMode(B_OP_BLEND);		
			Menu()->DrawBitmap(fBitmap,drawPoint);
			Menu()->SetDrawingMode(B_OP_OVER);
		}else
			Menu()->DrawBitmap(fBitmap,drawPoint);
		// offset to title point.
		drawPoint.y += ceil( fHeightDelta/2 );	
		drawPoint.x += 20;
		// Move draw point.
		Menu()->MovePenTo(drawPoint);
	}
	
	BMenuItem::DrawContent();
}

/***********************************************************
 * Extruct content width
 ***********************************************************/
void
IconMenuItem::GetContentSize(float *width, float *height)
{
	BMenuItem::GetContentSize(width,height);
	(*width) += 20;
	fHeightDelta = 16 - (*height);
	if( (*height) < 16)
		(*height) = 16;
}

/***********************************************************
 * Set the other bitmap.
 ***********************************************************/
void
IconMenuItem::SetBitmap(BBitmap *bitmap)
{
	delete fBitmap;
	fBitmap = bitmap;
}

