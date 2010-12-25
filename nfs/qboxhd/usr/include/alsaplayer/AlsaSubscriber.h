/*  AlsaSubscriber.h
 *  Copyright (C) 1998 Andy Lo A Foe <andy@alsaplayer.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *
 *  $Id: AlsaSubscriber.h,v 1.2 2002/04/20 11:59:19 adnans Exp $
*/ 

#ifndef __AlsaSubscriber_h__
#define __AlsaSubscriber_h__

#include "AlsaNode.h"
#include <pthread.h>


class AlsaSubscriber
{
 private:
	AlsaNode *the_node;
	int the_ID;
	int preferred_pos;
 public:
	AlsaSubscriber();
	~AlsaSubscriber();
	void Subscribe(AlsaNode *node, int pos=POS_BEGIN);
	void Unsubscribe();
	void EnterStream(streamer_type str, void *arg);
	void ExitStream();	
};

#endif
