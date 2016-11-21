
#pragma once


#include <ADL/MsgCommonStruct.h>


// Interface for NLS
struct INLSObject
	//interface INLSObject
{
	STDMETHOD_(LONG, NLSGetKeyValue)() = 0;
	STDMETHOD_(BOOL, NLSGetRoomID)(RoomID & roomID) = 0;
	STDMETHOD_(void, NLSSetErrorCode)(LONG lErrCode) = 0;
};
typedef INLSObject * LPNLSOBJECT;
typedef LPNLSOBJECT * LPLPNLSOBJECT;