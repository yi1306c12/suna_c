#
#       (XCS)
#	------------------------------------
#	Learning Classifier System based on accuracy
#
#     by Martin Butz
#     University of Wuerzburg / University of Illinois at Urbana/Champaign
#     butz@illigal.ge.uiuc.edu
#     Last modified: 10-17-99
#
#     Makefile
#


CC = gcc
CC_OPTS = -O2
LINKFINAL = -lm

ENVIRONMENT = ArffEnv
#ENVIRONMENT = boolFktEnv
OUTPUTNAME = xcs1.2.1

$(OUTPUTNAME):	xcs.o classifierList.o $(ENVIRONMENT).o actionSelection.o treeRepresentation.o xcsMacros.o
		$(CC) $(CC_OPTS) -o $(OUTPUTNAME) $(LINKFINAL) xcs.o classifierList.o actionSelection.o $(ENVIRONMENT).o treeRepresentation.o xcsMacros.o

xcs.o:	        xcs.c xcs.h classifierList.h xcsMacros.h env.h actionSelection.h treeRepresentation.h
		$(CC) -c $(CC_OPTS) xcs.c 

classifierList.o:	classifierList.c classifierList.h xcsMacros.h env.h
			$(CC) -c $(CC_OPTS) classifierList.c

actionSelection.o:	actionSelection.c actionSelection.h xcsMacros.h env.h classifierList.h
			$(CC) -c $(CC_OPTS) actionSelection.c

treeRepresentation.o:   treeRepresentation.c treeRepresentation.h xcsMacros.h env.h classifierList.h actionSelection.h
			$(CC) -c $(CC_OPTS) treeRepresentation.c

$(ENVIRONMENT).o:	$(ENVIRONMENT).c $(ENVIRONMENT).h xcsMacros.h
			$(CC) -c $(CC_OPTS) $(ENVIRONMENT).c

xcsMacros.o:	        xcsMacros.c xcsMacros.h
			$(CC) -c $(CC_OPTS) xcsMacros.c

clean:
	-rm -f *.o *.out *~
