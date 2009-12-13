.NOTABS=yes

locals = -I./core -I./libs/cross/libgeotrans/src -I./libs/cross/ezxml -I./libs/cross/sdlgfx -I./libs/cross/ul  -I./libs/cross/nmeap
ficl = -I./libs/cross/ficl -I./libs/cross/ficl/ficlplatform
addons = -I/usr/include/libpng -I/usr/include -I/usr/include/SDL

#-I./gpsbabel/jeeps -I./libs/cross/sdlimage

defs = -DFICL_ANSI=1 -DEZXML_NOMMAP=1

cc	= gcc
cpp	= g++
includes= -I/usr/include $(locals) $(ficl) $(addons)
cflags	= -g -O3 $(includes) $(defs) `wx-config --cxxflags` 
ldflags = -lSDL -lSDL_image `wx-config --libs`

#	./libs/cross/sdlimage/IMG_bmp \
#	./libs/cross/sdlimage/IMG \
#	./libs/cross/sdlimage/IMG_gif \
#	./libs/cross/sdlimage/IMG_jpg \
#	./libs/cross/sdlimage/IMG_lbm \
#	./libs/cross/sdlimage/IMG_pcx \
#	./libs/cross/sdlimage/IMG_png \
#	./libs/cross/sdlimage/IMG_pnm \
#	./libs/cross/sdlimage/IMG_tga \
#	./libs/cross/sdlimage/IMG_tif \
#	./libs/cross/sdlimage/IMG_xcf \
#	./libs/cross/sdlimage/IMG_xpm \
#	./libs/cross/sdlimage/IMG_xv \
#	./libs/cross/sdlimage/IMG_xxx \

sources = ./mapwindow \
	./mapview \
	./ozex \
	./ozex_gps \
	./libs/cross/nmeap/nmeap01 \
	./libs/cross/libgeotrans/src/albers \
	./libs/cross/libgeotrans/src/azeq \
	./libs/cross/libgeotrans/src/bng \
	./libs/cross/libgeotrans/src/bonne \
	./libs/cross/libgeotrans/src/cassini \
	./libs/cross/libgeotrans/src/cyleqa \
	./libs/cross/libgeotrans/src/eckert4 \
	./libs/cross/libgeotrans/src/eckert6 \
	./libs/cross/libgeotrans/src/eqdcyl \
	./libs/cross/libgeotrans/src/gars \
	./libs/cross/libgeotrans/src/geocent \
	./libs/cross/libgeotrans/src/georef \
	./libs/cross/libgeotrans/src/gnomonic \
	./libs/cross/libgeotrans/src/grinten \
	./libs/cross/libgeotrans/src/lambert_1 \
	./libs/cross/libgeotrans/src/lambert_2 \
	./libs/cross/libgeotrans/src/loccart \
	./libs/cross/libgeotrans/src/mercator \
	./libs/cross/libgeotrans/src/mgrs \
	./libs/cross/libgeotrans/src/miller \
	./libs/cross/libgeotrans/src/mollweid \
	./libs/cross/libgeotrans/src/neys \
	./libs/cross/libgeotrans/src/nzmg \
	./libs/cross/libgeotrans/src/omerc \
	./libs/cross/libgeotrans/src/orthogr \
	./libs/cross/libgeotrans/src/polarst \
	./libs/cross/libgeotrans/src/polycon \
	./libs/cross/libgeotrans/src/sinusoid \
	./libs/cross/libgeotrans/src/stereogr \
	./libs/cross/libgeotrans/src/tranmerc \
	./libs/cross/libgeotrans/src/trcyleqa \
	./libs/cross/libgeotrans/src/ups \
	./libs/cross/libgeotrans/src/usng \
	./libs/cross/libgeotrans/src/utm \
	./libs/cross/ezxml/ezxml \
	./libs/cross/ul/ulClock \
	./libs/cross/ul/ul \
	./libs/cross/ul/ulError \
	./libs/cross/ul/ulLinkedList \
	./libs/cross/ul/ulList \
	./libs/cross/ul/ulRTTI \
	./core/ll_geometry \
	./core/file_collection \
	./core/ozf_decoder \
	./core/img_decoder \
	./core/map_loader \
	./core/map_projection \
	./core/gauss \
	./core/map_container \
	./core/wpt_container \
	./core/map_index \
	./core/map_filters \
	./core/map_datums \
	./core/map_render \
	./core/qsort \
	./core/log_stream \
	./libs/cross/sdlgfx/SDL_framerate \
	./libs/cross/sdlgfx/SDL_gfxBlitFunc \
	./libs/cross/sdlgfx/SDL_gfxPrimitives \
	./libs/cross/sdlgfx/SDL_imageFilter \
	./libs/cross/sdlgfx/SDL_rotozoom \
	./libs/cross/ficl/bit \
	./libs/cross/ficl/callback \
	./libs/cross/ficl/compatibility \
	./libs/cross/ficl/dictionary \
	./libs/cross/ficl/double \
	./libs/cross/ficl/extras \
	./libs/cross/ficl/fileaccess \
	./libs/cross/ficl/float \
	./libs/cross/ficl/hash \
	./libs/cross/ficl/lzcompress \
	./libs/cross/ficl/lzuncompress \
	./libs/cross/ficl/prefix \
	./libs/cross/ficl/primitives \
	./libs/cross/ficl/search \
	./libs/cross/ficl/softcore \
	./libs/cross/ficl/stack \
	./libs/cross/ficl/system \
	./libs/cross/ficl/tools \
	./libs/cross/ficl/utility \
	./libs/cross/ficl/vm \
	./libs/cross/ficl/word \
	./libs/cross/ficl/ficlplatform/ansi 		

objects = {$(sources)}.o

target = ./ozex

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
