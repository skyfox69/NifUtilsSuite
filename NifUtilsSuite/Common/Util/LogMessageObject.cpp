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
#include "Common\GUI\NifUtilsSuiteFrm.h"
#include "Common\GUI\LogWindow.h"
#include "NifUtilsSuite.h"
#include <sstream>

using namespace std;

//-----  STATICS  -------------------------------------------------------------
static	char*	clientTypeArray[] = {"Common", "NifConvert", "ChunkMerge", "ChunkExtract", "BlenderPrepare", "ModelViewer"};

vector<string>	LogMessageObject::_logBuffer;

//-----  LogMessageObject()  --------------------------------------------------
LogMessageObject::LogMessageObject(ClientType clientType)
	:	_clientType(clientType)
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
	Configuration*	pConfig   (Configuration::getInstance());
	CLogWindow*		pLogWindow(((CNifUtilsSuiteFrame*) theApp.m_pMainWnd)->GetLogWindow());
	char*			pStart    (cbuffer);
	CHARFORMAT		charFormat = {0};
	stringstream	sStream;
	int				tType     (type);

	//  special handling of type settings
	if (cbuffer[0] == '^')
	{
		tType = atoi(cbuffer+1);
		pStart += 2;
	}

	//  early return on non enabled type
	if (!pConfig->_lvwLogActive[tType])		return;

	//  append of newline necessary?
	if (cbuffer[strlen(cbuffer) - 1] != '\n')
	{
		strcat(pStart, "\r\n");
	}

	if (pLogWindow != NULL)
	{
		//  concat log text
		if (type == NCU_MSG_TYPE_IGN_CLIENT)
		{
			sStream << pStart;
		}
		else
		{
		sStream << clientTypeArray[_clientType] << ":: " << pStart;
		}

		//  character format
		charFormat.cbSize      = sizeof(charFormat);
		charFormat.dwMask      = CFM_COLOR;
		charFormat.dwEffects   = 0;
		charFormat.crTextColor = pConfig->_lvwColors[tType];

		//  send text to log window
		pLogWindow->LogMessage(CString(sStream.str().c_str()), &charFormat);
	}
	else
	{
		//  concat log text
		sStream << '^' << tType << clientTypeArray[_clientType] << ":: " << pStart;

		//  write into log buffer
		_logBuffer.push_back(sStream.str());
	}
}

//-----  DumpLogBuffer()  -----------------------------------------------------
void LogMessageObject::DumpLogBuffer()
{
	if (!_logBuffer.empty())
	{
		//  dump each row of log buffer
		for (auto pIter=_logBuffer.begin(),pEnd=_logBuffer.end(); pIter != pEnd; ++pIter)
		{
			LogMessage(NCU_MSG_TYPE_IGN_CLIENT, pIter->c_str());
		}

		//  clear log buffer
		_logBuffer.clear();
	}
}
