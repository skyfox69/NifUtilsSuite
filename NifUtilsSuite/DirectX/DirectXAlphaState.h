/**
 *  file:    DirectXAlphaState.h
 *  struct:  DirectXAlphaState
 *
 *  structure defining alpha state values for rendering DirectX
 *
 */

#pragma once

//-----  CLASS  ---------------------------------------------------------------
struct DirectXAlphaState
{
	DWORD		_source;
	DWORD		_destination;
	DWORD		_argument;
	DWORD		_threshold;
	DWORD		_function;
	bool		_enabledBlend;
	bool		_enabledTest;
	
	DirectXAlphaState() : _source      (0),
						  _destination (0),
						  _argument    (0),
						  _threshold   (0),
						  _function    (8),
						  _enabledBlend(true),
						  _enabledTest (true)
						{};

	DirectXAlphaState(DWORD source, DWORD destination, DWORD argument, DWORD threshold, DWORD function, bool enabledBlend, bool enabledTest)
																		: _source      (source),
																		  _destination (destination),
																		  _argument    (argument),
																		  _threshold   (threshold),
																		  _function    (function),
																		  _enabledBlend(enabledBlend),
																		  _enabledTest (enabledTest)
																		{};
	virtual ~DirectXAlphaState() {};
};
