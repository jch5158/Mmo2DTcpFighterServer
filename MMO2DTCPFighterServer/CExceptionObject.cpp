#include "stdafx.h"
#include "CExceptionObject.h"

// TODO : 유니코드로 바꾸기
CExceptionObject::CExceptionObject(char* buffer, int bufferSize, const char* data)
{
	memcpy(this->m_MessageLog, buffer, bufferSize);

	this->m_BufferSize = bufferSize;

	strcpy_s(this->m_ErrorDataLog, data);
}

CExceptionObject::~CExceptionObject()
{

	
}