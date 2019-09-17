AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -Wall -std=c++11 -DFORTIFY_SOURCE=2 -DGCRYPT_NO_DEPRECATED
AM_LDFLAGS = -Wl,-rpath=/lib/homegear -Wl,-rpath=/usr/lib/homegear -Wl,-rpath=/usr/local/lib/homegear
LIBS += -Wl,-Bdynamic

libdir = $(localstatedir)/lib/homegear/modules
lib_LTLIBRARIES = mod_mbus.la
mod_mbus_la_SOURCES = Crc16.cpp DescriptionCreator.cpp Mbus.cpp MbusPacket.cpp MbusPeer.cpp Factory.cpp GD.cpp MbusCentral.cpp Interfaces.cpp VifConverter.cpp PhysicalInterfaces/Hgdc.cpp PhysicalInterfaces/IMbusInterface.cpp PhysicalInterfaces/Amber.cpp
mod_mbus_la_LDFLAGS =-module -avoid-version -shared
install-exec-hook:
	rm -f $(DESTDIR)$(libdir)/mod_mbus.la
