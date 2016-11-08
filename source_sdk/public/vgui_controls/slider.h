//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef SLIDER_H
#define SLIDER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>

namespace vgui2
{

//-----------------------------------------------------------------------------
// Labeled horizontal slider
//-----------------------------------------------------------------------------
class Slider : public Panel
{
	DECLARE_CLASS_SIMPLE( Slider, Panel );

public:
	Slider(Panel *parent, const char *panelName);

	// interface
	virtual void SetValue(int value, bool bTriggerChangeMessage = true); 
	virtual int  GetValue();
    virtual void SetRange(int min, int max);	 // set to max and min range of rows to display
	virtual void GetRange(int &min, int &max);
	virtual void GetNobPos(int &min, int &max);	// get current Slider position
	virtual void SetButtonOffset(int buttonOffset);
	virtual void OnCursorMoved(int x, int y);
	virtual void OnMousePressed(MouseCode code);
	virtual void OnMouseDoublePressed(MouseCode code);
	virtual void OnMouseReleased(MouseCode code);
	virtual void SetTickCaptions(const wchar_t *left, const wchar_t *right);
	virtual void SetTickCaptions(const char *left, const char *right);
	virtual void SetNumTicks(int ticks);
	virtual void SetThumbWidth( int width );
	
	// If you click on the slider outside of the nob, the nob jumps
	// to the click position, and if this setting is enabled, the nob
	// is then draggable from the new position until the mouse is released
	virtual void SetDragOnRepositionNob( bool state );
	virtual bool IsDragOnRepositionNob() const;

protected:
	virtual void OnSizeChanged(int wide, int tall);
	virtual void Paint();
	virtual void PaintBackground();
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(IScheme *pScheme);
	virtual void GetSettings(KeyValues *outResourceData);
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual const char *GetDescription();
	virtual void OnKeyCodeTyped(KeyCode code);

	virtual void DrawNob();
	virtual void DrawTicks();
	virtual void DrawTickLabels();

	virtual void GetTrackRect( int &x, int &y, int &w, int &h );

protected:
	virtual void RecomputeNobPosFromValue();
	virtual void RecomputeValueFromNobPos();
	virtual void SendSliderMovedMessage();

	bool _dragging;
	int _nobPos[2];
	int _nobDragStartPos[2];
	int _dragStartPos[2];
	int _range[2];
	int _value;		// the position of the Slider, in coordinates as specified by SetRange/SetRangeWindow
	int _buttonOffset;
	IBorder *_sliderBorder;
	IBorder *_insetBorder;
	float _nobSize;

	TextImage *_leftCaption;
	TextImage *_rightCaption;

	SDK_Color m_TickColor;
	SDK_Color m_TrackColor;
	SDK_Color m_DisabledTextColor1;
	SDK_Color m_DisabledTextColor2;

	int		m_nNumTicks;
	bool	m_bIsDragOnRepositionNob;
};

}

#endif // SLIDER_H
