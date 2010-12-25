/*
 * $Id: mosaic_descriptor.h,v 1.3 2005/10/29 00:10:08 obi Exp $
 *
 * Copyright (C) 2002-2005 Andreas Oberritter <obi@saftware.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * See the file 'COPYING' in the top level directory for details.
 */

#ifndef __mosaic_descriptor_h__
#define __mosaic_descriptor_h__

#include "descriptor.h"

class ElementaryCellField
{
	protected:
#ifndef DUOLABS
		unsigned elementaryCellId			: 6;
#else
		uint8_t elementaryCellId;
#endif

	public:
		ElementaryCellField(const uint8_t * const buffer);

		uint8_t getElementaryCellId(void) const;
};

typedef std::list<ElementaryCellField *> ElementaryCellFieldList;
typedef ElementaryCellFieldList::iterator ElementaryCellFieldIterator;
typedef ElementaryCellFieldList::const_iterator ElementaryCellFieldConstIterator;

class MosaicCell
{
	protected:
#ifndef DUOLABS
		unsigned logicalCellId				: 6;
		unsigned logicalCellPresentationInfo		: 3;
		unsigned elementaryCellFieldLength		: 8;
#else
		uint8_t logicalCellId;
		uint8_t logicalCellPresentationInfo;
		uint8_t elementaryCellFieldLength;
#endif
		ElementaryCellFieldList elementaryCellFields;
#ifndef DUOLABS
		unsigned cellLinkageInfo			: 8;
		unsigned bouquetId				: 16;
		unsigned originalNetworkId			: 16;
		unsigned transportStreamId			: 16;
		unsigned serviceId				: 16;
		unsigned eventId				: 16;
#else
		uint8_t cellLinkageInfo;
		uint16_t bouquetId;
		uint16_t originalNetworkId;
		uint16_t transportStreamId;
		uint16_t serviceId;
		uint16_t eventId;
#endif

	public:
		MosaicCell(const uint8_t * const buffer);
		~MosaicCell(void);

		uint8_t getLogicalCellId(void) const;
		uint8_t getLogicalCellPresentationInfo(void) const;
		const ElementaryCellFieldList *getElementaryCellFields(void) const;
		uint8_t getCellLinkageInfo(void) const;
		uint16_t getBouquetId(void) const;
		uint16_t getOriginalNetworkId(void) const;
		uint16_t getTransportStreamId(void) const;
		uint16_t getServiceId(void) const;
		uint16_t getEventId(void) const;
};

typedef std::list<MosaicCell *> MosaicCellList;
typedef MosaicCellList::iterator MosaicCellIterator;
typedef MosaicCellList::const_iterator MosaicCellConstIterator;

class MosaicDescriptor : public Descriptor
{
	protected:
#ifndef DUOLABS
		unsigned mosaicEntryPoint			: 1;
		unsigned numberOfHorizontalElementaryCells	: 3;
		unsigned numberOfVerticalElementaryCells	: 3;
#else
		uint8_t mosaicEntryPoint;
		uint8_t numberOfHorizontalElementaryCells;
		uint8_t numberOfVerticalElementaryCells;
#endif
		MosaicCellList mosaicCells;

	public:
		MosaicDescriptor(const uint8_t * const buffer);
		~MosaicDescriptor(void);

		uint8_t getMosaicEntryPoint(void) const;
		uint8_t getNumberOfHorizontalElementaryCells(void) const;
		uint8_t getNumberOfVerticalElementaryCells(void) const;
		const MosaicCellList *getMosaicCells(void) const;
};

#endif /* __mosaic_descriptor_h__ */
