# vim: ts=4:expandtab

APP_NAME	    ?= lingualeo
APP_VERSION     ?= 1.0.2
APP_RELEASE     ?= 1

QMAKE		    ?= qmake-qt5
SRC_DIR		    ?= ./src/
SPEC_FILE	    ?= ./$(APP_NAME).spec
RPMS_DIR        ?= ./rpms/
DESTDIR         ?= ./dest/
TARGET	        ?= $(SRC_DIR)/$(APP_NAME)
VERSION_FILE    ?= $(SRC_DIR)/version.h

.PHONY: default rpm clean install $(TARGET)

default: $(TARGET)

$(TARGET):
	echo "#ifndef VERSION_HPP"	                    >  $(VERSION_FILE)
	echo "#define VERSION_HPP"                      >> $(VERSION_FILE)
	echo "#define APP_VERSION \"$(APP_VERSION)\""   >> $(VERSION_FILE)
	echo "#define APP_RELEASE \"$(APP_RELEASE)\""   >> $(VERSION_FILE)
	echo "#endif"                                   >> $(VERSION_FILE)
	cd $(SRC_DIR) && $(QMAKE)
	$(MAKE) -C $(SRC_DIR)

clean:
	$(MAKE) -C $(SRC_DIR) $@
	rm -f $(SRC_DIR)/.qmake.stash
	rm -f $(TARGET)

install:
	## compile
	$(MAKE)
	## make dirs
	mkdir   -p $(DESTDIR)/usr/bin/
	mkdir   -p $(DESTDIR)/usr/share/$(APP_NAME)/images/
	mkdir   -p $(DESTDIR)/usr/share/applications/
	## copy files
	cp      -f $(TARGET)            $(DESTDIR)/usr/bin/$(APP_NAME)
	cp      -f $(SRC_DIR)/images/*  $(DESTDIR)/usr/share/$(APP_NAME)/images/
	cp      -f $(APP_NAME).desktop  $(DESTDIR)/usr/share/applications/

rpm: clean
	mkdir -p $(RPMS_DIR)
	rpmbuild -bb $(SPEC_FILE)                       \
        --define "package_path    $(PWD)"           \
        --define "package_version $(APP_VERSION)"   \
        --define "package_release $(APP_RELEASE)"   \
        --define "_rpmdir         $(RPMS_DIR)"

