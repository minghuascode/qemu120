
# mkf.mk derived from pibfile and cmake file
#   check CL_REFLECT_GEN_DIRECTORY ???


#$(warning .VARIABLES $(.VARIABLES))
#always define TOPROOT for sub build files to use 
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


.phony: all clean 

all: $(BUILDDIRS) $(BUILDEXEDIRS)
$(BUILDDIRS):
	@echo '================================================'
	@echo DIR: build $(SRCDIRPREFIX)/$(@:build-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:build-dirs-%=%) && \
	        make SRCDIR=$(@:build-dirs-%=%) -f $(TOPROOT)/mkfsub.mk allsub
	@echo 
$(BUILDEXEDIRS):
	@echo '================================================'
	@echo DIR: link $(SRCDIRPREFIX)/$(@:buildexe-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:buildexe-dirs-%=%) && \
	        make SRCDIR=$(@:buildexe-dirs-%=%) -f $(TOPROOT)/mkfsub.mk allsubexe
	@echo 

clean: $(CLEANDIRS)
$(CLEANDIRS):
	@echo '================================================'
	@echo DIR: clean $(SRCDIRPREFIX)/$(@:clean-dirs-%=%)
	@cd $(SRCDIRPREFIX)/$(@:clean-dirs-%=%) && \
	        make SRCDIR=$(@:clean-dirs-%=%) -f $(TOPROOT)/mkfsub.mk cleansub
	@echo 


