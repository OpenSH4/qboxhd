#include <lib/gui/elistbox.h>
#include <lib/gui/elistboxcontent.h>
#include <lib/gui/eslider.h>
#include <lib/actions/action.h>

eListbox::eListbox(eWidget *parent) :
	eWidget(parent), m_scrollbar_mode(showNever), m_prev_scrollbar_page(-1),
	m_content_changed(false), m_enabled_wrap_around(false), m_top(0), m_selected(0), m_itemheight(25),
	m_items_per_page(0), m_selection_enabled(1), m_scrollbar(NULL)
{
	memset(&m_style, 0, sizeof(m_style));
//	setContent(new eListboxStringContent());

	ePtr<eActionMap> ptr;
	eActionMap::getInstance(ptr);
	ptr->bindAction("ListboxActions", 0, 0, this);
}

eListbox::~eListbox()
{
	if (m_scrollbar)
		delete m_scrollbar;

	ePtr<eActionMap> ptr;
	eActionMap::getInstance(ptr);
	ptr->unbindAction(this, 0);
}

void eListbox::setScrollbarMode(int mode)
{
	m_scrollbar_mode = mode;
	if (m_scrollbar)
	{
		if (m_scrollbar_mode == showNever)
		{
			delete m_scrollbar;
			m_scrollbar=0;
		}
	}
	else
	{
		m_scrollbar = new eSlider(this);
		m_scrollbar->hide();
		m_scrollbar->setBorderWidth(1);
		m_scrollbar->setOrientation(eSlider::orVertical);
		m_scrollbar->setRange(0,100);
	}
}

void eListbox::setWrapAround(bool state)
{
	m_enabled_wrap_around = state;
}

void eListbox::setContent(iListboxContent *content)
{
	m_content = content;
	if (content)
		m_content->setListbox(this);
	entryReset();
}

bool eListbox::atBegin()
{
	if (m_content && !m_selected)
		return true;
	return false;
}

bool eListbox::atEnd()
{
	if (m_content && m_content->size() == m_selected+1)
		return true;
	return false;
}

void eListbox::moveToEnd()
{
	if (!m_content)
		return;
	/* move to last existing one ("end" is already invalid) */
	m_content->cursorEnd(); m_content->cursorMove(-1);
	/* current selection invisible? */
	if (m_top + m_items_per_page <= m_content->cursorGet())
	{
		int rest = m_content->size() % m_items_per_page;
		if (rest)
			m_top = m_content->cursorGet() - rest + 1;
		else
			m_top = m_content->cursorGet() - m_items_per_page + 1;
		if (m_top < 0)
			m_top = 0;
	}
}

void eListbox::moveSelection(long dir)
{
		/* refuse to do anything without a valid list. */
	if (!m_content)
		return;
		/* if our list does not have one entry, don't do anything. */
	if (!m_items_per_page)
		return;
		/* we need the old top/sel to see what we have to redraw */
	int oldtop = m_top;
	int oldsel = m_selected;
		/* first, move cursor */
	switch (dir)
	{
	case moveUp:
	{
		m_content->cursorMove(-1);
		if (m_enabled_wrap_around && oldsel == m_content->cursorGet())  // must wrap around ?
			moveToEnd();
		break;
	}
	case moveDown:
		m_content->cursorMove(1);
			/* ok - we could have reached the end. So we do wrap around. */
		if (!m_content->cursorValid())
		{
			if (m_enabled_wrap_around)
			{
				m_top = 0;
				m_content->cursorHome();
			}
			else
				m_content->cursorMove(-1);
		}
		break;
	case pageUp:
		if (m_content->cursorGet() >= m_items_per_page)
		{
			m_content->cursorMove(-m_items_per_page);
			m_top -= m_items_per_page;
			if (m_top < 0)
				m_top = 0;
		} else
		{
			m_top = 0;
			m_content->cursorHome();
		}
		break;
	case moveTop:
		m_content->cursorHome();
		m_top = 0; /* align with top, speeds up process */
		break;
	case pageDown:
		m_content->cursorMove(m_items_per_page);
		if (m_content->cursorValid())
			break;
				/* fall through */
	case moveEnd:
		moveToEnd();
		break;
	case justCheck:
		break;
	}

	if (m_content->cursorValid() && !m_content->currentCursorSelectable())
	{
			/* ok, our cursor position is valid (i.e. in list), but not selectable. */

			/* when moving up, continue until we found a valid position. */
		if ((dir == moveUp) || (dir == pageDown))
		{
			while (m_content->cursorGet())
			{
				m_content->cursorMove(-1);
				if (m_content->currentCursorSelectable())
				{
					break;
				}
			}
		} else
		{
				/* else move down */
			while (m_content->cursorValid())
			{
				m_content->cursorMove(+1);
				if (m_content->currentCursorSelectable())
				{
					break;
				}
			}

			if (!m_content->cursorValid())
				m_content->cursorMove(-1);
		}

		if (!m_content->currentCursorSelectable())
			m_content->cursorSet(oldsel);
	}
		/* note that we could be on an invalid cursor position, but we don't
		   care. this only happens on empty lists, and should have almost no
		   side effects. */

		/* now, look wether the current selection is out of screen */
	m_selected = m_content->cursorGet();
	while (m_selected < m_top)
	{
		m_top -= m_items_per_page;
		if (m_top < 0)
			m_top = 0;
	}

	while (m_selected >= m_top + m_items_per_page)
		/* m_top should be always valid here as it's selected */
		m_top += m_items_per_page;

	if (oldsel != m_selected)
		/* emit */ selectionChanged();
	updateScrollBar();
	if (m_top != oldtop)
		invalidate();
	else if (m_selected != oldsel)
	{
   /* redraw the old and newly selected */
		gRegion inv = eRect(0, m_itemheight * (m_selected-m_top), size().width(), m_itemheight);
		inv |= eRect(0, m_itemheight * (oldsel-m_top), size().width(), m_itemheight);
		invalidate(inv);
	}
}

void eListbox::moveSelectionTo(int index)
{
	if (m_content)
	{
		m_content->cursorHome();
		m_content->cursorMove(index);
		moveSelection(justCheck);
	}
}

int eListbox::getCurrentIndex()
{
	if (m_content && m_content->cursorValid())
		return m_content->cursorGet();
	return 0;
}

void eListbox::updateScrollBar()
{
	if (!m_content || m_scrollbar_mode == showNever )
		return;
	int entries = m_content->size();
	if (m_content_changed)
	{
		int width = size().width();
		int height = size().height();
		m_content_changed = false;
		if (entries > m_items_per_page || m_scrollbar_mode == showAlways)
		{
			int sbarwidth=width/16;
			if (sbarwidth < 18)
				sbarwidth = 18;
			if (sbarwidth > 22)
				sbarwidth = 22;
			m_scrollbar->move(ePoint(width-sbarwidth, 0));
			m_scrollbar->resize(eSize(sbarwidth, height));
			m_content->setSize(eSize(width-sbarwidth-5, m_itemheight));
			m_scrollbar->show();
		}
		else
		{
			m_content->setSize(eSize(width, m_itemheight));
			m_scrollbar->hide();
		}
	}
	if (m_items_per_page && entries)
	{
		int curVisiblePage = m_top / m_items_per_page;
		if (m_prev_scrollbar_page != curVisiblePage)
		{
			m_prev_scrollbar_page = curVisiblePage;
			int pages = entries / m_items_per_page;
			if ((pages*m_items_per_page) < entries)
				++pages;
			int start=(m_top*100)/(pages*m_items_per_page);
			int vis=(m_items_per_page*100+pages*m_items_per_page-1)/(pages*m_items_per_page);
			if (vis < 3)
				vis=3;
			m_scrollbar->setStartEnd(start,start+vis);
		}
	}
}

int eListbox::getEntryTop()
{
	return (m_selected - m_top) * m_itemheight;
}

int eListbox::event(int event, void *data, void *data2)
{
#if 0
  char *val[]={ "evtPaint",
                "evtKey",
                "evtChangedPosition",
                "evtChangedSize",

                "evtParentChangedPosition",

                "evtParentVisibilityChanged",
                "evtWillChangePosition", /* new size is eRect *data */
                "evtWillChangeSize",

                "evtAction",

                "evtFocusGot",
                "evtFocusLost",

                "evtUserWidget",
                ""};

  eDebug(" eListbox::event <<<<<<<<<<<<< event=%d - %s ----> ",event, val[event]);
  printTs("");
#endif ///0

	switch (event)
	{
	case evtPaint:
	{
		ePtr<eWindowStyle> style;

		if (!m_content)
			return eWidget::event(event, data, data2);
		ASSERT(m_content);

		getStyle(style);

		if (!m_content)
			return 0;

		gPainter &painter = *(gPainter*)data2;

		m_content->cursorSave();
		m_content->cursorMove(m_top - m_selected);

		gRegion entryrect = eRect(0, 0, size().width(), m_itemheight);
		const gRegion &paint_region = *(gRegion*)data;

		for (int y = 0, i = 0; i <= m_items_per_page; y += m_itemheight, ++i)
		{
			gRegion entry_clip_rect = paint_region & entryrect;

			if (!entry_clip_rect.empty())
				m_content->paint(painter, *style, ePoint(0, y), m_selected == m_content->cursorGet() && m_content->size() && m_selection_enabled);

				/* (we could clip with entry_clip_rect, but
				   this shouldn't change the behavior of any
				   well behaving content, so it would just
				   degrade performance without any gain.) */
			m_content->cursorMove(+1);
			entryrect.moveBy(ePoint(0, m_itemheight));
		}

		// clear/repaint empty/unused space between scrollbar and listboxentrys
		if (m_scrollbar && m_scrollbar->isVisible())
		{
			style->setStyle(painter, eWindowStyle::styleListboxNormal);
			painter.clip(eRect(m_scrollbar->position() - ePoint(5,0), eSize(5,m_scrollbar->size().height())));
			painter.clear();
			painter.clippop();
		}

		m_content->cursorRestore();

		return 0;
	}

	case evtChangedSize:
		recalcSize();
		return eWidget::event(event, data, data2);

	case evtAction:
		if (isVisible())
		{
			moveSelection((long)data2);
			return 1;
		}
		return 0;
	default:
		return eWidget::event(event, data, data2);
	}
}

void eListbox::recalcSize()
{
	m_content_changed=true;
	m_prev_scrollbar_page=-1;
	if (m_content)
		m_content->setSize(eSize(size().width(), m_itemheight));
	m_items_per_page = size().height() / m_itemheight;

	if (m_items_per_page < 0) /* TODO: whyever - our size could be invalid, or itemheigh could be wrongly specified. */
 		m_items_per_page = 0;

	moveSelection(justCheck);
}

void eListbox::setItemHeight(int h)
{
	if (h)
		m_itemheight = h;
	else
		m_itemheight = 20;
	recalcSize();
}

void eListbox::setSelectionEnable(int en)
{
	if (m_selection_enabled == en)
		return;
	m_selection_enabled = en;
	entryChanged(m_selected); /* redraw current entry */
}

void eListbox::entryAdded(int index)
{
	if (m_content && (m_content->size() % m_items_per_page) == 1)
		m_content_changed=true;
	/* manage our local pointers. when the entry was added before the current position, we have to advance. */

		/* we need to check <= - when the new entry has the (old) index of the cursor, the cursor was just moved down. */
	if (index <= m_selected)
		++m_selected;
	if (index <= m_top)
		++m_top;

		/* we have to check wether our current cursor is gone out of the screen. */
		/* moveSelection will check for this case */
	moveSelection(justCheck);

		/* now, check if the new index is visible. */
	if ((m_top <= index) && (index < (m_top + m_items_per_page)))
	{
			/* todo, calc exact invalidation... */
		invalidate();
	}
}

void eListbox::entryRemoved(int index)
{
	if (m_content && !(m_content->size() % m_items_per_page))
		m_content_changed=true;

	if (index == m_selected && m_content)
		m_selected = m_content->cursorGet();

	if (m_content && m_content->cursorGet() >= m_content->size())
		moveSelection(moveUp);
	else
		moveSelection(justCheck);

	if ((m_top <= index) && (index < (m_top + m_items_per_page)))
	{
			/* todo, calc exact invalidation... */
		invalidate();
	}
}

void eListbox::entryChanged(int index)
{
	if ((m_top <= index) && (index < (m_top + m_items_per_page)))
	{
		gRegion inv = eRect(0, m_itemheight * (index-m_top), size().width(), m_itemheight);
		invalidate(inv);
	}
}

void eListbox::entryReset(bool selectionHome)
{
	m_content_changed = true;
	m_prev_scrollbar_page = -1;
	int oldsel;

	if (selectionHome)
	{
		if (m_content)
			m_content->cursorHome();
		m_top = 0;
		m_selected = 0;
	}

	if (m_content && (m_selected >= m_content->size()))
	{
		if (m_content->size())
			m_selected = m_content->size() - 1;
		else
			m_selected = 0;
		m_content->cursorSet(m_selected);
	}

	oldsel = m_selected;
	moveSelection(justCheck);
		/* if oldsel != m_selected, selectionChanged was already
		   emitted in moveSelection. we want it in any case, so otherwise,
		   emit it now. */
	if (oldsel == m_selected)
		/* emit */ selectionChanged();
	invalidate();
}

void eListbox::setBackgroundColor(gRGB &col)
{
	m_style.m_background_color = col;
	m_style.m_background_color_set = 1;
}

void eListbox::setBackgroundColorSelected(gRGB &col)
{
	m_style.m_background_color_selected = col;
	m_style.m_background_color_selected_set = 1;
}

void eListbox::setForegroundColor(gRGB &col)
{
	m_style.m_foreground_color = col;
	m_style.m_foreground_color_set = 1;
}

void eListbox::setForegroundColorSelected(gRGB &col)
{
	m_style.m_foreground_color_selected = col;
	m_style.m_foreground_color_selected_set = 1;
}

void eListbox::setBackgroundPicture(ePtr<gPixmap> &pm)
{
	m_style.m_background = pm;
}

void eListbox::setSelectionPicture(ePtr<gPixmap> &pm)
{
	m_style.m_selection = pm;
}

void eListbox::invalidate(const gRegion &region)
{
	gRegion tmp(region);
	if (m_content)
		m_content->updateClip(tmp);
	eWidget::invalidate(tmp);
}

struct eListboxStyle *eListbox::getLocalStyle(void)
{
		/* transparency is set directly in the widget */
	m_style.m_transparent_background = isTransparent();
	return &m_style;
}
