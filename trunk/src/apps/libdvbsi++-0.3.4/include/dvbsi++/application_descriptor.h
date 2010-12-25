/*
 * $Id: application_descriptor.h,v 1.3 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2004-2005 St�phane Est�-Gracias <sestegra@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */
 
#ifndef __application_descriptor_h__
#define __application_descriptor_h__

#include "application_profile.h"
#include "descriptor.h"

typedef std::list<uint8_t> TransportProtocolLabelList;
typedef TransportProtocolLabelList::iterator TransportProtocolLabelIterator;
typedef TransportProtocolLabelList::const_iterator TransportProtocolLabelConstIterator;

class ApplicationDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned applicationProfilesLength		:8;
#else
		uint8_t applicationProfilesLength;
#endif
		ApplicationProfileList applicationProfiles;
#ifndef DUOLABS
		unsigned serviceBoundFlag			:1;
		unsigned visibility					:2;
		unsigned applicationPriority		:8;
#else
		uint8_t serviceBoundFlag;
		uint8_t visibility;
		uint8_t applicationPriority;
#endif
		TransportProtocolLabelList transportProtocolLabels;
	
	public:
		ApplicationDescriptor(const uint8_t * const buffer);
		~ApplicationDescriptor(void);

		const ApplicationProfileList *getApplicationProfiles(void) const;
		uint8_t getServiceBoundFlag(void) const;
		uint8_t getVisibility(void) const;
		uint8_t getApplicationPriority(void) const;
		const TransportProtocolLabelList *getTransportProtocolLabels(void) const;
};

#endif /* __application_descriptor_h__ */
