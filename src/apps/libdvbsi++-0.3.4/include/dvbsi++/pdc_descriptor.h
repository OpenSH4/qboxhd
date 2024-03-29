/*
 * $Id: pdc_descriptor.h,v 1.2 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2002-2005 Andreas Oberritter <obi@saftware.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __pdc_descriptor_h__
#define __pdc_descriptor_h__

#include "descriptor.h"

class PdcDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned programmeIdentificationLabel		: 20;
#else
		uint32_t programmeIdentificationLabel;
#endif

	public:
		PdcDescriptor(const uint8_t * const buffer);

		uint32_t getProgrammeIdentificationLabel(void) const;
};

#endif /* __pcd_descriptor_h__ */
