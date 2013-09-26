/*************************************************************
*   IconMenu
*
*   Toolbar is a usefull UI component.
*
*   @author  Atsushi Takamatsu (tak_atsu@tau.bekkoame.ne.jp)
**************************************************************/

#ifndef __ICONMENUITEM_H__
#define __ICONMENUITEM_H__

#include <Be.h>

class IconMenuItem :public BMenuItem {
public:
					IconMenuItem(const char* label,BMessage *message,
							char shortcut = 0,uint32 modifiers = 0,BBitmap *bitmap = NULL);
	virtual			~IconMenuItem();
			void	SetBitmap(BBitmap *bitmap);
protected:
	virtual void	DrawContent();
	virtual void	GetContentSize(float *width ,float *height);
private:
		BBitmap 	*fBitmap;
		float 		fHeightDelta;
};
#endif