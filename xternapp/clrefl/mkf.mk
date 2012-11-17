
# mkf.mk edited from pibfile
#   check CL_REFLECT_GEN_DIRECTORY


#$(warning .VARIABLES $(.VARIABLES))
export TOPROOT := $(CURDIR)


SRCDIRS := clReflectCpp
SRCDIRS += clReflectUtil
SRCDIRS += clReflectCore

EXEDIRS := clReflectScan
EXEDIRS += clReflectMerge
EXEDIRS += clReflectExport
EXEDIRS += clReflectTest

SRCDIRS += $(EXEDIRS)


SRCDIRPREFIX := src

BUILDDIRS := $(SRCDIRS:%=build-dirs-%)
CLEANDIRS := $(SRCDIRS:%=clean-dirs-%)
BUILDEXEDIRS := $(EXEDIRS:%=buildexe-dirs-%)


.phony: all clean prelink

all: prelink $(BUILDDIRS) $(BUILDEXEDIRS)
$(BUILDDIRS):
	@echo '================================================'
	@echo DIR: $(SRCDIRPREFIX)/$(@:build-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:build-dirs-%=%) && \
	        make SRCDIR=$(@:build-dirs-%=%) -f $(TOPROOT)/mkfsub.mk allsub
	@echo 
$(BUILDEXEDIRS):
	@echo '================================================'
	@echo DIR: $(SRCDIRPREFIX)/$(@:buildexe-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:buildexe-dirs-%=%) && \
	        make SRCDIR=$(@:buildexe-dirs-%=%) -f $(TOPROOT)/mkfsub.mk allsubexe
	@echo 

clean: $(CLEANDIRS)
$(CLEANDIRS):
	@echo '================================================'
	@echo DIR: $(SRCDIRPREFIX)/$(@:clean-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:clean-dirs-%=%) && \
	        make SRCDIR=$(@:clean-dirs-%=%) -f $(TOPROOT)/mkfsub.mk cleansub
	@echo 

prelink:
	#if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi


