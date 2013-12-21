/**
 *  file:   LogMessageObject.h
 *  class:  LogMessageObject
 *
 *  Class defining default functionality as log message client
 *
 */

#pragma once

//-----  INCLUDES  ------------------------------------------------------------
#include "Common\Util\DefLogMessageTypes.h"
#include <vector>
#include <string>

using namespace std;

//-----  CLASS  ---------------------------------------------------------------
class LogMessageObject
{
	public:
		enum ClientType
		{
			COMMON = 0,
			NIFCONVERT = 1,
			CHUNKMERGE = 2,
			CHUNKEXTRACT = 3,
			BLENDERPREPARE = 4,
			MODELVIEWER = 5
		};

	private:
		ClientType				_clientType;
		static	vector<string>	_logBuffer;

	public:
								LogMessageObject(ClientType clientType);
		virtual					~LogMessageObject();

		virtual	void			LogMessage(const int type, const char* pMessage, ...);
		virtual	void			DumpLogBuffer();
};
