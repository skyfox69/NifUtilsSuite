/**
 *  file:   IfcBroadcastObject.h
 *  class:  IfcBroadcastObject
 *
 *  Abstract class defining interface as broadcast client
 *
 */

#pragma once

#define		IBCE_CHANGED_SETTINGS		1000
#define		IBCE_SET_TOOLTIPP			1001
#define		IBCE_SHOW_MODEL				1002
#define		IBCE_LOG_WINDOW_KILLED		1003
#define		IBCE_AUTO_SET_PATH			1004


class IfcBroadcastObject
{
	public:
		virtual	BOOL			BroadcastEvent(WORD event, void* pParameter=NULL) = 0;
};
