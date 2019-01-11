
//
//		OsDependent
//		BouKiCHi 2019
//		(OS依存のルーチンをまとめます)
//		onion software/onitama 2019/1
//

#include <stdio.h>
#include "osdep.h"

static int MUCOM88IF_VM_COMMAND(int cmd, int prm1, int prm2, void *prm3, void *prm4)
{
	return 0;
}


static int MUCOM88IF_EDITOR_COMMAND(int cmd, int prm1, int prm2, void *prm3, void *prm4)
{
	return 0;
}


OsDependent::OsDependent(void)
{
}

OsDependent::~OsDependent(void)
{
}

int OsDependent::AddPlugins(char *filename)
{
	//		プラグインを追加する
	//		filename = プラグインDLL名
	//
	Mucom88Plugin *plg = new Mucom88Plugin;
	plg->if_mucomvm = (MUCOM88IF_COMMAND)MUCOM88IF_VM_COMMAND;
	plg->if_editor = (MUCOM88IF_COMMAND)MUCOM88IF_EDITOR_COMMAND;
	plugins.push_back(plg);
	return InitPlugin(plg, filename);
}


void OsDependent::FreePlugins(void)
{

}


void OsDependent::NoticePlugins(int cmd)
{

}


