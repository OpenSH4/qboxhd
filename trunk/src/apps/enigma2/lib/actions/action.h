#ifndef __lib_driver_action_h
#define __lib_driver_action_h

#include <lib/base/object.h>

		/* avoid warnigs :) */
#include <features.h>
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#include <lib/python/python.h>
#include <string>
#include <map>

class eWidget;

SWIG_IGNORE(eActionMap);
class eActionMap: public iObject
{
	DECLARE_REF(eActionMap);
#ifdef SWIG
	eActionMap();
	~eActionMap();
#endif
public:
#ifndef SWIG
	eActionMap();
	~eActionMap();
	void bindAction(const std::string &context, int priority, int id, eWidget *widget);
	void unbindAction(eWidget *widget, int id);
#endif

	void bindAction(const std::string &context, int priority, SWIG_PYOBJECT(ePyObject) function);
	void unbindAction(const std::string &context, SWIG_PYOBJECT(ePyObject) function);

#ifdef QBOXHD
	void bindKey(const std::string &domain, const std::string &device, const std::string &rc_id, int key, int flags, int repeat_for_event, const std::string &context, const std::string &action);
	void keyPressed(const std::string &device, const std::string &rc_id, int key, int flags);
	std::string getLastRCId(void);
#else
	void bindKey(const std::string &domain, const std::string &device, int key, int flags, const std::string &context, const std::string &action);
	void keyPressed(const std::string &device, int key, int flags);
#endif
	void unbindKeyDomain(const std::string &domain);
	
#ifndef SWIG
	static RESULT getInstance(ePtr<eActionMap> &);
private:
#ifdef QBOXHD	
	std::string last_rc_id;
	int m_repeat_counter;       // counter of repeater
#endif

	static eActionMap *instance;
	struct eActionBinding
	{
		eActionBinding()
			:m_prev_seen_make_key(-1)
		{}
//		eActionContext *m_context;
		std::string m_context; // FIXME
		std::string m_domain;
		
		ePyObject m_fnc;
		
		eWidget *m_widget;
		int m_id;
		int m_prev_seen_make_key;
	};
	
	std::multimap<int, eActionBinding> m_bindings;

	friend struct compare_string_keybind_native;
	struct eNativeKeyBinding
	{
		std::string m_device;
#ifdef QBOXHD
		std::string m_rc_id;
		int m_repeat_for_event;     // max repeater for generate a event. If it is 0 every repeat generate a event.
#endif
		std::string m_domain;
		int m_key;
		int m_flags;
		
//		eActionContext *m_context;
		int m_action;
	};
	
	std::multimap<std::string, eNativeKeyBinding> m_native_keys;
	
	friend struct compare_string_keybind_python;
	struct ePythonKeyBinding
	{
		std::string m_device;
#ifdef QBOXHD
		std::string m_rc_id;
		int m_repeat_for_event;     // max repeater for generate a event. If it is 0 every repeat generate a event.
#endif
		std::string m_domain;
		int m_key;
		int m_flags;
		
		std::string m_action;
	};
	
	std::multimap<std::string, ePythonKeyBinding> m_python_keys;
#endif
};
SWIG_TEMPLATE_TYPEDEF(ePtr<eActionMap>, eActionMap);
SWIG_EXTEND(ePtr<eActionMap>,
	static ePtr<eActionMap> getInstance()
	{
		extern ePtr<eActionMap> NewActionMapPtr(void);
		return NewActionMapPtr();
	}
);

#endif
