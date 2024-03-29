/*
 * $Id: extended_event_descriptor.h,v 1.3 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2002-2005 Andreas Oberritter <obi@saftware.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __extended_event_descriptor_h__
#define __extended_event_descriptor_h__

#include "descriptor.h"

class ExtendedEvent
{
	protected:
#ifndef DUOLABS
		unsigned itemDescriptionLength			: 8;
#else
		uint8_t itemDescriptionLength;
#endif
		std::string itemDescription;
#ifndef DUOLABS
		unsigned itemLength				: 8;
#else
		uint8_t itemLength;
#endif
		std::string item;

	public:
		ExtendedEvent(const uint8_t * const buffer);

		const std::string &getItemDescription(void) const;
		const std::string &getItem(void) const;

	friend class ExtendedEventDescriptor;
};

typedef std::list<ExtendedEvent *> ExtendedEventList;
typedef ExtendedEventList::iterator ExtendedEventIterator;
typedef ExtendedEventList::const_iterator ExtendedEventConstIterator;

class ExtendedEventDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned descriptorNumber			: 4;
		unsigned lastDescriptorNumber			: 4;
#else
		uint8_t descriptorNumber;
		uint8_t lastDescriptorNumber;
#endif
		std::string iso639LanguageCode;
#ifndef DUOLABS
		unsigned lengthOfItems				: 8;
#else
		uint8_t lengthOfItems;
#endif
		ExtendedEventList items;
#ifndef DUOLABS
		unsigned textLength				: 8;
#else
		uint8_t textLength;
#endif
		std::string text;

	public:
		ExtendedEventDescriptor(const uint8_t * const buffer);
		~ExtendedEventDescriptor(void);

		uint8_t getDescriptorNumber(void) const;
		uint8_t getLastDescriptorNumber(void) const;
		const std::string &getIso639LanguageCode(void) const;
		const ExtendedEventList *getItems(void) const;
		const std::string &getText(void) const;
};

#endif /* __extended_event_descriptor_h__ */
