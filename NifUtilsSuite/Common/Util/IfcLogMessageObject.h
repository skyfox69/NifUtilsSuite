/**
 *  file:   IfcLogMessageObject.h
 *  class:  IfcLogMessageObject
 *
 *  Abstract class defining interface as log message client
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\DefLogMessageTypes.h"

//-----  INTERFACE  -----------------------------------------------------------
class IfcLogMessageObject
{
	public:
		virtual	void			LogMessage(const CString text, const CHARFORMAT* pFormat) = 0;
};
