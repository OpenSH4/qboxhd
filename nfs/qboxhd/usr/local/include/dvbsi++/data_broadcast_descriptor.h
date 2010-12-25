/*
 * $Id: data_broadcast_descriptor.h,v 1.3 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2002-2005 Andreas Oberritter <obi@saftware.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __data_broadcast_descriptor_h__
#define __data_broadcast_descriptor_h__

#include "descriptor.h"

typedef std::list<uint8_t> selectorByteList;
typedef selectorByteList::iterator selectorByteIterator;
typedef selectorByteList::const_iterator selectorByteConstIterator;

class DataBroadcastDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned dataBroadcastId			: 16;
		unsigned componentTag				: 8;
		unsigned selectorLength				: 8;
#else
		uint16_t dataBroadcastId;
		uint8_t componentTag;
		uint8_t selectorLength;
#endif
		selectorByteList selectorBytes;
		std::string iso639LanguageCode;
#ifndef DUOLABS
		unsigned textLength				: 8;
#else
		uint8_t textLength;
#endif
		std::string text;

	public:
		DataBroadcastDescriptor(const uint8_t * const buffer);

		uint16_t getDataBroadcastId(void) const;
		uint8_t getComponentTag(void) const;
		const selectorByteList *getSelectorBytes(void) const;
		const std::string &getIso639LanguageCode(void) const;
		const std::string &getText(void) const;
};

#endif /* __data_broadcast_descriptor_h__ */
