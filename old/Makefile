
all: api tests finish
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	CFLAGS = -std=c++11 -O3 -ffast-math -msse3 -I/usr/include/postgresql -fPIC -I/usr/include/boost -Wno-deprecated-register
	LFLAGS = -l pthread -lpq -lboost_timer -lboost_thread -lboost_system -lboost_iostreams -lboost_log_setup -lboost_log -lboost_date_time -lboost_filesystem
	CC = g++-4.9
	DIR = ./linux
endif

ifeq ($(UNAME), Darwin)
	CFLAGS = -DBOOST_LOG_DYN_LINK -std=c++11 -stdlib=libc++ -O3 -fvectorize -ffast-math -fslp-vectorize-aggressive -msse3 -I/usr/local/Cellar/boost/1.54.0/include/ -ferror-limit=3 -Wno-deprecated-register
	LFLAGS = -DBOOST_LOG_DYN_LINK -lpq -std=c++11 -L/usr/local/Cellar/boost/1.54.0/lib/ -stdlib=libc++ -lboost_timer-mt -lboost_thread-mt -lboost_system-mt -lboost_iostreams-mt -lboost_log_setup-mt -lboost_log-mt -lboost_date_time-mt -lboost_filesystem-mt
	CC = clang++
	DIR = ./darwin
endif


API = qcatfield.o qcatcondition.o qcat.o qcatdatasource.o qcatbin.o qcatbinnumeric.o qcatbintimestamp.o qcatbinpassthrough.o qcatfieldstats.o qcatattribute.o qcatbinstats.o qcatpqresult.o qcatngram.o qcatbinstrategy.o qcatfieldmanager.o

TESTS = sanity_test.o

api: $(API)
	$(CC) $(API) $(LFLAGS) $(CFLAGS) -shared -o qcatlib.o
	
tests: $(API) $(TESTS)
	$(CC) $(TESTS) $(API) $(LFLAGS) $(CFLAGS) -o tests/run_tests

finish: 
	mkdir -p $(DIR)
	mv *.o $(DIR)
	
sanity_test.o: tests/sanity_test.cpp
	$(CC) -c $(CFLAGS) tests/sanity_test.cpp 

qcatngram.o: ../src/qcatngram.cpp
	$(CC) -c $(CFLAGS) ../src/qcatngram.cpp ../src/qcatngram.h

qcatdatasource.o: ../src/qcatdatasource.cpp 
	$(CC) -c $(CFLAGS) ../src/qcatdatasource.cpp 

qcatfield.o: ../src/qcatfield.cpp
	$(CC) -c $(CFLAGS) ../src/qcatfield.cpp  

qcatattribute.o: ../src/qcatattribute.cpp
	$(CC) -c $(CFLAGS) ../src/qcatattribute.cpp  

qcatbin.o: ../src/qcatbin.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbin.cpp  

qcatbinstrategy.o: ../src/qcatbinstrategy.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbinstrategy.cpp  

qcatbinnumeric.o: ../src/qcatbinnumeric.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbinnumeric.cpp

qcatbintimestamp.o: ../src/qcatbintimestamp.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbintimestamp.cpp

qcatbinpassthrough.o: ../src/qcatbinpassthrough.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbinpassthrough.cpp

qcatfieldstats.o: ../src/qcatfieldstats.cpp
	$(CC) -c $(CFLAGS) ../src/qcatfieldstats.cpp

qcatbinstats.o: ../src/qcatbinstats.cpp
	$(CC) -c $(CFLAGS) ../src/qcatbinstats.cpp

qcatcondition.o: ../src/qcatcondition.cpp
	$(CC) -c $(CFLAGS) ../src/qcatcondition.cpp

qcatfieldmanager.o: ../src/qcatfieldmanager.cpp
	$(CC) -c $(CFLAGS) ../src/qcatfieldmanager.cpp

qcat.o: ../src/qcat.cpp
	$(CC) -c $(CFLAGS) ../src/qcat.cpp 

qcatpqresult.o: ../src/qcatpqresult.cpp
	$(CC) -c $(CFLAGS) ../src/qcatpqresult.cpp

clean:
	rm -rf *.o

