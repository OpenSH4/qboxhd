/*
 * $Id: prefetch_descriptor.h,v 1.3 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2004-2005 Stéphane Esté-Gracias <sestegra@free.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */
 
#ifndef __prefetch_descriptor_h__
#define __prefetch_descriptor_h__

#include "descriptor.h"

class PrefetchLabel
{
	protected:
#ifndef DUOLABS
		unsigned labelLength				: 8;
#else
		uint8_t labelLength;
#endif
		std::string label;
#ifndef DUOLABS
		unsigned prefetchPriority			: 8;
#else
		uint8_t prefetchPriority;
#endif

	public:
		PrefetchLabel(const uint8_t * const buffer);

		uint8_t getLabelLength(void) const;
		const std::string &getLabel(void) const;
		uint8_t getPrefetchPriority(void) const;
};

typedef std::list<PrefetchLabel *> PrefetchLabelList;
typedef PrefetchLabelList::iterator PrefetchLabelIterator;
typedef PrefetchLabelList::const_iterator PrefetchLabelConstIterator;

class PrefetchDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned transportProtocolLabel			: 8;
#else
		uint8_t transportProtocolLabel;
#endif
		PrefetchLabelList prefetchLabels;

	public:
		PrefetchDescriptor(const uint8_t * const buffer);
		~PrefetchDescriptor(void);
		
		uint8_t getTransportProtocolLabel(void) const;
		const PrefetchLabelList *getPrefetchLabels(void) const;
};

#endif /* __prefetch_descriptor_h__ */
