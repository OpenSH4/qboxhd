/*
 * $Id: group_link_descriptor.h,v 1.2 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2004-2005 St�phane Est�-Gracias <sestegra@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __group_link_descriptor_h__
#define __group_link_descriptor_h__

#include "descriptor.h"

class GroupLinkDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned position				: 8;
		unsigned groupId				: 32;
#else
		uint8_t position;
		uint32_t groupId;
#endif

	public:
		GroupLinkDescriptor(const uint8_t * const buffer);

		uint8_t getPosition(void) const;
		uint32_t getGroupId(void) const;
};

#endif /* __group_link_descriptor_h__ */
