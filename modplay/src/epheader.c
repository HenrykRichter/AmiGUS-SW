/* **************************************************************************

   (C) 2024 Henryk Richter

   AGUS_Mod eagleplayer plugin

   ************************************************************************** */
#include <misc/EaglePlayer.h>

extern struct TagItem plugin_tags[];

//static const  struct    DeliTrackerPlayer ply;
static const  struct	DeliTrackerPlayer ply = {
	0x70FF4e75,
//	{0x44,0x45,0x4c,0x49,0x52,0x49,0x55,0x4D}, /* "DELIRIUM", */
	{0x45,0x50,0x50,0x4C,0x41,0x59,0x45,0x52}, /* "EPPLAYER", */
	plugin_tags
};


