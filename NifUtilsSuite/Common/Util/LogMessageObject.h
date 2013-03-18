/**
 *  file:   LogMessageObject.h
 *  class:  LogMessageObject
 *
 *  Class defining default functionality as log message client
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\IfcLogMessageObject.h"

//-----  CLASS  ---------------------------------------------------------------
class LogMessageObject : public IfcLogMessageObject
{
	public:
								LogMessageObject();
		virtual					~LogMessageObject();

		virtual	void			LogMessage(const int type, const char* pMessage, ...);

		//  IfcLogMessageObject
		virtual	void			LogMessage(const CString text, const CHARFORMAT* pFormat) = 0;
};
