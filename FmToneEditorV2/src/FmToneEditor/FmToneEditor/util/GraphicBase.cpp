#include "GraphicBase.h"


GraphicBase::GraphicBase()
{
	m_iWidth = 0;
	m_iHeight = 0;
	m_lPitch = 0;
	m_pData = NULL;
	m_dColorKey = 0xffffffff;
}

GraphicBase::~GraphicBase()
{
}
