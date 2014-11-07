
include config.mk


CFILES 	:= $(shell find $(TOP)/src -type f -name "*.c")
CXXFILES:= $(shell find $(TOP)/src -type f -name "*.cpp")
AFILES	:= $(shell find $(TOP)/src -type f -name "*.s")
HFILES	:= $(shell find $(TOP)/src -type f -name "*.h")

SFILES 	:= $(CFILES) $(CXXFILES) $(AFILES)
OFILES	:= $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(AFILES:.s=.o)


.PHONY: all clean git

all: iso

aplus : $(OFILES)
	@echo "  LD      " $@
	@$(LD) $(LFLAGS) -o $@ $(OFILES) $(LIBS)
	@echo "  OBJCPY  " aplus.sym
	@$(OBJCPY) --only-keep-debug $@ aplus.sym
	@echo "  STRIP   " $@
	@$(OBJCPY) --strip-debug $@ $@
	@echo "  ZIP     " apluz
	$(CP) $@ $(PREFIX)/$@
	$(ZIP) $(PREFIX)/$@
	$(MV) $(PREFIX)/$@.gz $(PREFIX)/apluz

.c.o:
	@echo "  CC      " $(notdir $<)
	@$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	@echo "  CXX     " $(notdir $<)
	@$(CXX) $(CXXFLAGS) $< -o $@

.s.o:
	@echo "  ASM     " $(notdir $<)
	@$(ASM) $(AFLAGS) $< -o $@
	

iso: aplus
	grub-mkrescue $(TOP)/bin -o $(TOP)/aplus.iso
	qemu-system-i386 -m 64 -serial stdio -cdrom aplus.iso -net nic,model=rtl8139 -net user,hostfwd=udp::5500-:5500 -net dump


clean:
	-@$(RM) $(OFILES)
	
git: clean
	-@git add --all .
	-@git commit -m "$(COMMIT)"
	-@git push origin master
	
	
