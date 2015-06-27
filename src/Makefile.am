
bin_PROGRAMS = libqcat

AM_CXXFLAGS = $(INTI_CFLAGS)

libqcat_CPPFLAGS = $(BOOST_CPPFLAGS)
libqcat_LDFLAGS = $(BOOST_THREAD_LDFLAGS)
libqcat_LTLIBRARIES = $(BOOST_THREAD_LIBS)

libqcat_SOURCES = main.cc helloworld.cc helloworld.h
libqcat_LDADD = -lboost_timer-mt -lboost_thread-mt -lboost_system-mt -lboost_iostreams-mt -lboost_log_setup-mt -lboost_log-mt -lboost_date_time-mt -lboost_filesystem-mt
