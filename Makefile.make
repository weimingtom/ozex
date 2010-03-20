.PHONY : clean

locals = -I./core -I./libs/cross/libgeotrans/src -I./libs/cross/ezxml -I./libs/cross/sdlgfx -I./libs/cross/sdlimage -I./libs/cross/ul  -I./libs/cross/nmeap -I./libs/cross/zlib
ficl = -I./libs/cross/ficl -I./libs/cross/ficl/ficlplatform
addons = -I/usr/include/libpng -I/usr/include

defs = -DFICL_ANSI=1 -DEZXML_NOMMAP=1

cc	= gcc
cpp	= g++
includes= -I/usr/include $(locals) $(ficl) $(addons)
cflags	= -g -ggdb -W -Wall -pipe $(includes) $(defs) $(shell wx-config --cppflags) $(shell sdl-config --cflags)
ldflags = -lz -lSDL_image $(shell wx-config --libs) $(shell sdl-config --libs)

objects = ./mapwindow.o \
	./mapview.o \
	./ozex.o \
	./ozex_gps.o \
	./libs/cross/nmeap/nmeap01.o \
	./libs/cross/libgeotrans/src/albers.o \
	./libs/cross/libgeotrans/src/azeq.o \
	./libs/cross/libgeotrans/src/bng.o \
	./libs/cross/libgeotrans/src/bonne.o \
	./libs/cross/libgeotrans/src/cassini.o \
	./libs/cross/libgeotrans/src/cyleqa.o \
	./libs/cross/libgeotrans/src/eckert4.o \
	./libs/cross/libgeotrans/src/eckert6.o \
	./libs/cross/libgeotrans/src/eqdcyl.o \
	./libs/cross/libgeotrans/src/gars.o \
	./libs/cross/libgeotrans/src/geocent.o \
	./libs/cross/libgeotrans/src/georef.o \
	./libs/cross/libgeotrans/src/gnomonic.o \
	./libs/cross/libgeotrans/src/grinten.o \
	./libs/cross/libgeotrans/src/lambert_1.o \
	./libs/cross/libgeotrans/src/lambert_2.o \
	./libs/cross/libgeotrans/src/loccart.o \
	./libs/cross/libgeotrans/src/mercator.o \
	./libs/cross/libgeotrans/src/mgrs.o \
	./libs/cross/libgeotrans/src/miller.o \
	./libs/cross/libgeotrans/src/mollweid.o \
	./libs/cross/libgeotrans/src/neys.o \
	./libs/cross/libgeotrans/src/nzmg.o \
	./libs/cross/libgeotrans/src/omerc.o \
	./libs/cross/libgeotrans/src/orthogr.o \
	./libs/cross/libgeotrans/src/polarst.o \
	./libs/cross/libgeotrans/src/polycon.o \
	./libs/cross/libgeotrans/src/sinusoid.o \
	./libs/cross/libgeotrans/src/stereogr.o \
	./libs/cross/libgeotrans/src/tranmerc.o \
	./libs/cross/libgeotrans/src/trcyleqa.o \
	./libs/cross/libgeotrans/src/ups.o \
	./libs/cross/libgeotrans/src/usng.o \
	./libs/cross/libgeotrans/src/utm.o \
	./libs/cross/ezxml/ezxml.o \
	./libs/cross/ul/ulClock.o \
	./libs/cross/ul/ul.o \
	./libs/cross/ul/ulError.o \
	./libs/cross/ul/ulLinkedList.o \
	./libs/cross/ul/ulList.o \
	./libs/cross/ul/ulRTTI.o \
	./core/ll_geometry.o \
	./core/file_collection.o \
	./core/ozf_decoder.o \
	./core/img_decoder.o \
	./core/map_loader.o \
	./core/map_projection.o \
	./core/gauss.o \
	./core/map_container.o \
	./core/wpt_container.o \
	./core/map_index.o \
	./core/map_filters.o \
	./core/map_datums.o \
	./core/map_render.o \
	./core/qsort.o \
	./core/log_stream.o \
	./libs/cross/sdlgfx/SDL_framerate.o \
	./libs/cross/sdlgfx/SDL_gfxBlitFunc.o \
	./libs/cross/sdlgfx/SDL_gfxPrimitives.o \
	./libs/cross/sdlgfx/SDL_imageFilter.o \
	./libs/cross/sdlgfx/SDL_rotozoom.o \
	./libs/cross/ficl/bit.o \
	./libs/cross/ficl/callback.o \
	./libs/cross/ficl/compatibility.o \
	./libs/cross/ficl/dictionary.o \
	./libs/cross/ficl/double.o \
	./libs/cross/ficl/extras.o \
	./libs/cross/ficl/fileaccess.o \
	./libs/cross/ficl/float.o \
	./libs/cross/ficl/hash.o \
	./libs/cross/ficl/lzcompress.o \
	./libs/cross/ficl/lzuncompress.o \
	./libs/cross/ficl/prefix.o \
	./libs/cross/ficl/primitives.o \
	./libs/cross/ficl/search.o \
	./libs/cross/ficl/softcore.o \
	./libs/cross/ficl/stack.o \
	./libs/cross/ficl/system.o \
	./libs/cross/ficl/tools.o \
	./libs/cross/ficl/utility.o \
	./libs/cross/ficl/vm.o \
	./libs/cross/ficl/word.o \
	./libs/cross/ficl/ficlplatform/ansi.o

target = ./ozex

all : $(target)

$(target): $(objects)
	$(cpp) $(ldflags)  -o $@ $(objects)

%.o : %.cpp
	$(cpp) $(cflags) -c $< -o $@

%.o: %.c
	$(cc) $(cflags) -c $< -o $@

ozex.o: ozex.h ozex_gps.h
mapview.o: mapview.h ozex.h
mapwindow.o: mapwindow.h ozex.h
ozex_gps.o: ozex_gps.h

clean:
	rm -f $(objects) $(target)
