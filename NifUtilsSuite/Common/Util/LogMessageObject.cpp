/**
 *  file:   LogMessageObject.cpp
 *  class:  LogMessageObject
 *
 *  Class defining default functionality as log message client
 *
 */

//-----  INCLUDES  ------------------------------------------------------------
#include "stdafx.h"
#include "Common\Util\LogMessageObject.h"
#include "Common\Util\Configuration.h"

//-----  LogMessageObject()  --------------------------------------------------
LogMessageObject::LogMessageObject()
{}

//-----  ~LogMessageObject()  -------------------------------------------------
LogMessageObject::~LogMessageObject()
{}

//-----  LogMessage()  --------------------------------------------------------
void LogMessageObject::LogMessage(const int type, const char* pMessage, ...)
{
	//  decode var-args
	char	cbuffer[5000] = {0};
	va_list	ap;

	va_start(ap, pMessage);
	vsnprintf(cbuffer, 5000, pMessage, ap);
	va_end(ap);

	//  start logging
	Configuration*	pConfig(Configuration::getInstance());
	CHARFORMAT		charFormat = {0};
	CString			text (cbuffer);
	int				tType(type);

	//  special handling of type settings
	if (cbuffer[0] == '^')
	{
		tType = atoi(cbuffer+1);
		text  = (cbuffer+2);
	}

	//  early return on non enabled type
	if (!pConfig->_lvwLogActive[tType])		return;

	//  append of newline necessary?
	if (cbuffer[strlen(cbuffer) - 1] != '\n')
	{
		text += _T("\r\n");
	}

	//  character format
	charFormat.cbSize      = sizeof(charFormat);
	charFormat.dwMask      = CFM_COLOR;
	charFormat.dwEffects   = 0;
	charFormat.crTextColor = pConfig->_lvwColors[tType];

	//  write into log view
	LogMessage(text, &charFormat);
}
