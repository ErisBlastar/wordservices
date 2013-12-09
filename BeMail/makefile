#-------------------------------------------------------
#
#	Generic Cross-platform Makefile
#	R3 x86 and ppc binaries
#
#	Author:	stephen beaulieu <hippo@be.com>
#
#	Questions, comments & concerns to <devsupport@be.com>
#
#	Copyright 1998 Be, Inc.  All Rights Reserved
#
#	Fill out the Project Specific Information section
#	Use a \ to continue information on different lines
#	example: SRCS = source1.cpp \
#					source2.cpp
#
#	When done simply cd to the directory in the terminal
#	and type make.  The objects and binary will appear
#	in an obj.x86 or obj.ppc folder.
#
#--------------------------------------------------------

#--------------------------------------------------------
#	Project Specific Information
#--------------------------------------------------------

#	specify the name of the project
	IMG_NAME	= BeMail
	
#	specify the type of image to create
#		APP:	Application
#		SHARED:	Shared Library
#		STATIC:	Static Library
	IMG_TYPE	= APP
	
#	specify the list of source files
	SRCS		=	\
					Mail.cpp		\
					Header.cpp		\
					Content.cpp		\
					Enclosures.cpp	\
					Prefs.cpp		\
					Signature.cpp	\
					Status.cpp		\
					FindWindow.cpp	\
					Utilities.c

#	specify the list of resource files
#	the names listed will automatically have
#	_ppc.rsrc or _x86.rsrc added to the end
#	depending on the platform
	RSRCS		= BeMail
	
#	specify any additional beos shared libraries needed.
#	these libraries should be specified by their
#	actual runtime name.  the makefile will add
# 	the appropriate path and any extention necessary.
#	libbe.so and libroot.so are automatically included
	ADD_BESHLIBS	= \
						libmail.so	\
						libtracker.so \

#	specify additional non-beos shared libraries needed.
#	the path for these libraries is figured from
#	the current directory.  the make file will
#	add any extention necessary.
	ADD_SHLIBS		= 

#	specify any additional beos static libraries needed.
#	these libraries should be specified by their
#	actual runtime name.  the makefile will add
# 	the appropriate path and any extention necessary.
	ADD_BESTLIBS	=

#	specify additional non-beos static libraries needed.
#	the path for these libraries is figured from
#	the current directory.  the make file will
#	add any extention necessary.
	ADD_STLIBS		=

#	specify any additional system paths to check
#	for files or headers to include in the project
#	these paths are specified from /boot/system/headers
#	most if not all headers under this path are automatically
#	included.  Add them specificly if you run into
#	problems
	SYSTEM_INCLUDES	=

#	specify any additional local paths to check
#	for files or headers to include in the project
#	these are determined from the current directory
	LOCAL_INCLUDES	=

#	All other settings are determined automatically
#	based on the platform

#--------------------------------------------------------
#	Project Independent Information
#--------------------------------------------------------

#	determine the CPU if not specified on the command line
ifndef CPU
	MACHINE =$(shell uname -m)
ifeq ($(MACHINE), BePC)
	CPU = x86
else
	CPU = ppc
endif
endif


#	set the full directory variable id not specified
ifeq ($(FULL_DIR),)
	FULL_DIR	:= $(shell pwd)
endif

#	set the object directory
	OBJ				:= obj.$(CPU)
	OBJ_DIR			:= obj.$(CPU)

#	specify the directory for libraries
	BELIBRARIES		=	/boot/develop/lib/$(CPU)

#	specify the default libraries
	DEFAULT_LIBS	=	\
						libbe.so	\
						libroot.so

#	specify the MIMESET tool
	MIMESET			= mimeset
	
#	specify the path to the headers
	BEHEADERS		= /boot/develop/headers

#	specify the compiler
	CC				= mwcc$(CPU)

#	set initial compiler flags
	CFLAGS			+= -g

#	specify the linker
	LD				= mwld$(CPU)

#	set the inital linker flags depending on IMG_TYPE
ifeq ($(IMG_TYPE), APP)
	LDFLAGS		+=	-xma
endif

ifeq ($(IMG_TYPE), SHARED)
	LDFLAGS		+=	-xms
endif

ifeq ($(IMG_TYPE), STATIC)
	LDFLAGS		+=	-xml
endif

#	specify basic linker flags
	LDFLAGS			+= \
						-nodefaults \
						$(BELIBRARIES)/glue-noinit.a \
						$(BELIBRARIES)/init_term_dyn.o \
						$(BELIBRARIES)/start_dyn.o 


#	platform specific settings
ifeq ($(CPU), x86)

#		specify the library extention
		LIB_EXTENTION	= .LIB

#		specify the optimizer setting
		ifndef OPTIMIZER
			OPTIMIZER 	= -O3
		endif

#		specify additional compiler flags
		CFLAGS			+= -inline off

#		specify additional linker flags
		LDFLAGS			+=  
		
else

ifeq ($(CPU), ppc)
#		specify the library extention
		LIB_EXTENTION	=

#		specify the optimizer setting
		ifndef OPTIMIZER
			OPTIMIZER 	= -O7
		endif

#		specify additional compiler flags
		CFLAGS			+= 

#		specify additional linker flags
		LDFLAGS			+=  \
							-export pragma \
							-init _init_routine_ \
							-term _term_routine_

endif
endif

#	specify the tool to add resources
	ADDRES			= copyres

#	set the Archiver tool and flags
	AR		= $(LD)
	ARFLAGS	+= -xml -o

#	create the final list of libraries to include
	LIBS_TO_USE =	$(addsuffix $(LIB_EXTENTION), $(ADD_SHLIBS) $(DEFAULT_LIBS) $(ADD_BESHLIBS)) $(ADD_STLIBS) $(ADD_BESTLIBS)
	

#	additional common linker flags
	LDFLAGS	+= -L$(FULL_DIR) -L$(BELIBRARIES) $(LIBS_TO_USE)
	
#	create the list of include paths
	INCLUDES = -i . $(addprefix -i ,$(LOCAL_INCLUDES)) -i- $(addprefix -i $(BEHEADERS)/,$(SYSTEM_INCLUDES)) 

#	create the list of resources
	RSRCS_TO_USE :=	$(addsuffix _$(CPU).rsrc, $(RSRCS))

#	specify where to create the application binary
	TARGET		:=$(OBJ_DIR)/$(IMG_NAME)

#	create the resource instruction
	ifeq ($(RSRCS), )
		DO_RSRCS :=
	else
		DO_RSRCS := $(ADDRES) $(RSRCS_TO_USE) $(TARGET)
	endif

# psuedo-function for converting a list of source files in SRCS variable
# to a corresponding list of object files in $(OBJ_DIR)/xxx.o
# The "function" strips off the src file suffix (.ccp or .c or whatever)
# and then strips of the directory name, leaving just the root file name.
# It then appends the .o suffix and prepends the $(OBJ_DIR)/ path
define SRCS_LIST_TO_OBJS
	$(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(foreach file, $(SRCS), $(basename $(notdir $(file))))))
endef


#	specify the list of objects
	OBJS		:= $(SRCS_LIST_TO_OBJS)

#	generate mapfiles for metrowerks.  
	SYMBOL_FILE	:= $(TARGET).xMAP
	LDFLAGS		+= -map $(SYMBOL_FILE)


#	define the actual work to be done	
default: $(TARGET)

$(TARGET):	$(OBJ_DIR) $(OBJS) $(RSRCS_TO_USE)
		$(LD) -o $@ $(OBJS) $(LDFLAGS)
		$(DO_RSRCS)
		$(MIMESET) -f $@


#--------------------------------------------------------
#	Rules for the whole system
#--------------------------------------------------------

#	rule to create the object file directory if needed
$(OBJ_DIR)::
	@[ -d $(OBJ_DIR) ] || mkdir $(OBJ_DIR) > /dev/null 2>&1

#	default rule for take xxx.c files on compile into $(OBJ_DIR)/xxx.o
$(OBJ_DIR)/%.o : %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

#	default rule for take xxx.cpp files on compile into $(OBJ_DIR)/xxx.o
$(OBJ_DIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

#	empty rule. Things that depend on this rule will always get triggered
FORCE:

#	The generic clean command. Delete everything in the object folder.
clean :: FORCE
	-rm -rf $(OBJ_DIR)

#	remove just the application from the object folder
rmapp ::
	-rm -f $(TARGET)
