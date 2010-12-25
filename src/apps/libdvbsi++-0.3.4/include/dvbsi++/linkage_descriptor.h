/*
 * $Id: linkage_descriptor.h,v 1.5 2005/11/28 16:25:09 ghostrider Exp $
 *
 * Copyright (C) 2002-2005 Andreas Oberritter <obi@saftware.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __linkage_descriptor_h__
#define __linkage_descriptor_h__

#include "descriptor.h"

typedef std::vector<uint8_t> PrivateDataByteVector;
typedef PrivateDataByteVector::iterator PrivateDataByteIterator;
typedef PrivateDataByteVector::const_iterator PrivateDataByteConstIterator;

class LinkageDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned transportStreamId			: 16;
		unsigned originalNetworkId			: 16;
		unsigned serviceId				: 16;
		unsigned linkageType				: 8;
#else
		uint16_t transportStreamId;
		uint16_t originalNetworkId;
		uint16_t serviceId;
		uint8_t linkageType;
#endif
		PrivateDataByteVector privateDataBytes;
#ifndef DUOLABS
		unsigned handOverType				: 4;
		unsigned originType				: 1;
		unsigned networkId				: 16;
		unsigned initialServiceId			: 16;
#else
		uint8_t handOverType;
		uint8_t originType;
		uint16_t networkId;
		uint16_t initialServiceId;
#endif

	public:
		LinkageDescriptor(const uint8_t * const buffer);

		uint16_t getTransportStreamId(void) const;
		uint16_t getOriginalNetworkId(void) const;
		uint16_t getServiceId(void) const;
		uint8_t getLinkageType(void) const;
		const PrivateDataByteVector *getPrivateDataBytes(void) const;
		uint8_t getHandOverType(void) const;
		uint8_t getOriginType(void) const;
		uint16_t getNetworkId(void) const;
		uint16_t getInitialServiceId(void) const;
};

#endif /* __linkage_descriptor_h__ */
