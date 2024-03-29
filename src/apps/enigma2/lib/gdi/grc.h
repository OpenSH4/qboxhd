#ifndef __grc_h
#define __grc_h

/*
	gPainter ist die high-level version. die highlevel daten werden zu low level opcodes ueber
	die gRC-queue geschickt und landen beim gDC der hardwarespezifisch ist, meist aber auf einen
	gPixmap aufsetzt (und damit unbeschleunigt ist).
*/

// for debugging use:
// #define SYNC_PAINT
#undef SYNC_PAINT

#include <pthread.h>
#include <stack>
#include <list>

#include <string>
#include <lib/base/elock.h>
#include <lib/base/message.h>
#include <lib/gdi/erect.h>
#include <lib/gdi/gpixmap.h>
#include <lib/gdi/region.h>
#include <lib/gdi/gfont.h>


#ifndef QBOXHD
  //this switch enables graphical modification not supported by QBOXHD
  #define E2_LAST_MOD
#else
  #undef E2_LAST_MOD
#endif ///QBOXHD

#ifdef E2_LAST_MOD
 #include <lib/gdi/compositing.h>
#endif ///E2_LAST_MOD

class eTextPara;

class gDC;
struct gOpcode
{
	enum Opcode
	{
		renderText,
		renderPara,
		setFont,

		fill, fillRegion, clear,
		blit,

		setPalette,
		mergePalette,

		line,

		setBackgroundColor,
		setForegroundColor,

		setBackgroundColorRGB,
		setForegroundColorRGB,

		setOffset,

		setClip, addClip, popClip,

		flush,

		waitVSync,
		flip,
		notify,

		enableSpinner, disableSpinner, incrementSpinner,

		shutdown,

		setCompositing,
	} opcode;

	gDC *dc;
	union para
	{
		struct pfillRect
		{
			eRect area;
		} *fill;

		struct pfillRegion
		{
			gRegion region;
		} *fillRegion;

		struct prenderText
		{
			eRect area;
			char *text;
			int flags;
		} *renderText;

		struct prenderPara
		{
			ePoint offset;
			eTextPara *textpara;
		} *renderPara;

		struct psetFont
		{
			gFont *font;
		} *setFont;

		struct psetPalette
		{
			gPalette *palette;
		} *setPalette;

		struct pblit
		{
			gPixmap *pixmap;
			int flags;
			eRect position;
			eRect clip;
		} *blit;

		struct pmergePalette
		{
			gPixmap *target;
		} *mergePalette;

		struct pline
		{
			ePoint start, end;
		} *line;

		struct psetClip
		{
			gRegion region;
		} *clip;

		struct psetColor
		{
			gColor color;
		} *setColor;

		struct psetColorRGB
		{
			gRGB color;
		} *setColorRGB;

		struct psetOffset
		{
			ePoint value;
			int rel;
		} *setOffset;

#ifdef E2_LAST_MOD
		gCompositingData *setCompositing;
#endif ///E2_LAST_MOD
	} parm;
};

#define MAXSIZE 2048

		/* gRC is the singleton which controls the fifo and dispatches commands */
class gRC: public iObject, public Object
{
	DECLARE_REF(gRC);
	friend class gPainter;
	static gRC *instance;

#ifndef SYNC_PAINT
	static void *thread_wrapper(void *ptr);
	pthread_t the_thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
#endif
	void *thread();

	gOpcode queue[MAXSIZE];
	int rp, wp;

	eFixedMessagePump<int> m_notify_pump;
	void recv_notify(const int &i);

	ePtr<gDC> m_spinner_dc;
	int m_spinner_enabled;

	void enableSpinner();
	void disableSpinner();
#ifdef E2_LAST_MOD
	ePtr<gCompositingData> m_compositing;
#endif ///E2_LAST_MOD
public:
	gRC();
	virtual ~gRC();

	void submit(const gOpcode &o);

	Signal0<void> notify;

	void setSpinnerDC(gDC *dc) { m_spinner_dc = dc; }

	static gRC *getInstance();
};

	/* gPainter is the user frontend, which in turn sends commands through gRC */
class gPainter
{
	ePtr<gDC> m_dc;
	ePtr<gRC> m_rc;
	friend class gRC;

	gOpcode *beginptr;
	void begin(const eRect &rect);
	void end();
public:
	gPainter(gDC *dc, eRect rect=eRect());
	virtual ~gPainter();

	void setBackgroundColor(const gColor &color);
	void setForegroundColor(const gColor &color);

	void setBackgroundColor(const gRGB &color);
	void setForegroundColor(const gRGB &color);

	void setFont(gFont *font);
		/* flags only THESE: */
	enum
	{
			// todo, make mask. you cannot align both right AND center AND block ;)
		RT_HALIGN_LEFT = 0,  /* default */
		RT_HALIGN_RIGHT = 1,
		RT_HALIGN_CENTER = 2,
		RT_HALIGN_BLOCK = 4,

		RT_VALIGN_TOP = 0,  /* default */
		RT_VALIGN_CENTER = 8,
		RT_VALIGN_BOTTOM = 16,

		RT_WRAP = 32
	};
	void renderText(const eRect &position, const std::string &string, int flags=0);

	void renderPara(eTextPara *para, ePoint offset=ePoint(0, 0));

	void fill(const eRect &area);
	void fill(const gRegion &area);

	void clear();

	enum
	{
		BT_ALPHATEST = 1,
		BT_ALPHABLEND = 2,
		BT_SCALE = 4 /* will be automatically set by blitScale */
	};

	void blit(gPixmap *pixmap, ePoint pos, const eRect &clip=eRect(), int flags=0);
	void blitScale(gPixmap *pixmap, const eRect &pos, const eRect &clip=eRect(), int flags=0, int aflags = BT_SCALE);

	void setPalette(gRGB *colors, int start=0, int len=256);
	void setPalette(gPixmap *source);
	void mergePalette(gPixmap *target);

	void line(ePoint start, ePoint end);

	void setOffset(ePoint abs);
	void moveOffset(ePoint rel);
	void resetOffset();

	void resetClip(const gRegion &clip);
	void clip(const gRegion &clip);
	void clippop();

	void waitVSync();
	void flip();
	void notify();
#ifdef E2_LAST_MOD
	void setCompositing(gCompositingData *comp);
#endif ///E2_LAST_MOD
	void flush();
};

class gDC: public iObject
{
	DECLARE_REF(gDC);
protected:
	ePtr<gPixmap> m_pixmap;

	gColor m_foreground_color, m_background_color;
	gRGB m_foreground_color_rgb, m_background_color_rgb;
	ePtr<gFont> m_current_font;
	ePoint m_current_offset;

	std::stack<gRegion> m_clip_stack;
	gRegion m_current_clip;

	ePtr<gPixmap> m_spinner_saved, m_spinner_temp;
	ePtr<gPixmap> *m_spinner_pic;
	eRect m_spinner_pos;
	int m_spinner_num, m_spinner_i;
public:
	virtual void exec(gOpcode *opcode);
	gDC(gPixmap *pixmap);
	gDC();
	virtual ~gDC();
	gRegion &getClip() { return m_current_clip; }
	int getPixmap(ePtr<gPixmap> &pm) { pm = m_pixmap; return 0; }
	gRGB getRGB(gColor col);
	virtual eSize size() { return m_pixmap->size(); }
	virtual int islocked() { return 0; }

	void enableSpinner();
	void disableSpinner();
	void incrementSpinner();
	void setSpinner(eRect pos, ePtr<gPixmap> *pic, int len);
};

#endif
