INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src

SUBDIRS = Components Tools Screens Plugins

noinst_LIBRARIES = libenigma_python.a

libenigma_python_a_SOURCES = \
	python.cpp enigma_python_wrap.cxx connections.cpp

enigma_python_wrap.cxx: enigma_python.i
	swig -I$(top_srcdir)/ -c++ -python -O $(QBOXHDBOARD) -nortti -nothreads enigma_python.i
	python enigma_py_patcher.py

enigma.py: enigma_python_wrap.cxx

CLEANFILES = enigma.py enigma_python_wrap.cxx

installdir = $(LIBDIR)/enigma2/python

install_PYTHON = \
	enigma.py
