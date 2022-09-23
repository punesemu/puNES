desktopdir = $(datadir)/applications
desktop_DATA = com.github.punesemu.punes.desktop

public_icons = \
	hicolor_apps_16x16_com.github.punesemu.punes.png \
	hicolor_apps_32x32_com.github.punesemu.punes.png \
	hicolor_apps_48x48_com.github.punesemu.punes.png \
	hicolor_apps_256x256_com.github.punesemu.punes.png
	
EXTRA_DIST = \
	$(public_icons) \
	$(desktop_DATA)
	
###############################################################################

update-icon-cache:
	update-desktop-database $(DESTDIR)$(datadir)/applications || :

install-icons:
	for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

uninstall-icons:
	-for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		rm -f $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done;

install-data-local: install-icons

uninstall-local: uninstall-icons
