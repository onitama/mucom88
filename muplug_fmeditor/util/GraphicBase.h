#pragma once
#include <Windows.h>

class GraphicBase
{
public:
	int			m_iWidth;
	int			m_iHeight;
	LONG_PTR	m_lPitch;
	BYTE		*m_pData;
	DWORD		m_dColorKey;

public:
	GraphicBase();
	~GraphicBase();
};

