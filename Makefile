ifeq ($(STATIC),1)
  PKG_CONFIG_STATIC_FLAG = --static
  CXXFLAGS_ALL += -static
endif

CXXFLAGS_ALL += -MMD -MP -MF objects/$*.d $(shell pkg-config --cflags $(PKG_CONFIG_STATIC_FLAG) sdl2 vorbisfile vorbis) $(CXXFLAGS) 
LDFLAGS_ALL += $(LDFLAGS)
LIBS_ALL += $(shell pkg-config --libs $(PKG_CONFIG_STATIC_FLAG) sdl2 vorbisfile vorbis) -pthread $(LIBS)

SOURCES = \
  Nexus/Animation.cpp \
  Nexus/Audio.cpp \
  Nexus/Collision.cpp \
  Nexus/Debug.cpp \
  Nexus/Drawing.cpp \
  Nexus/Ini.cpp \
  Nexus/Input.cpp \
  Nexus/main.cpp \
  Nexus/Math.cpp \
  Nexus/Object.cpp \
  Nexus/Palette.cpp \
  Nexus/Player.cpp \
  Nexus/Reader.cpp \
  Nexus/RetroEngine.cpp \
  Nexus/Scene.cpp \
  Nexus/Script.cpp \
  Nexus/Sprite.cpp \
  Nexus/String.cpp \
  Nexus/Text.cpp \
  Nexus/Userdata.cpp \
  Nexus/Video.cpp

	  
ifeq ($(FORCE_CASE_INSENSITIVE),1)
  CXXFLAGS_ALL += -DFORCE_CASE_INSENSITIVE
  SOURCES += Nexus/fcaseopen.c
endif

ifeq ($(USE_HW_REN),1)
  CXXFLAGS_ALL += -DUSE_HW_REN
  LIBS_ALL += -lGL -lGLEW
endif

OBJECTS = $(SOURCES:%=objects/%.o)
DEPENDENCIES = $(SOURCES:%=objects/%.d)

all: bin/nexus

include $(wildcard $(DEPENDENCIES))

objects/%.o: %
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_ALL) -std=c++17 $< -o $@ -c

bin/nexus: $(OBJECTS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS_ALL) $(LDFLAGS_ALL) $^ -o $@ $(LIBS_ALL)

install: bin/nexus
	install -Dp -m755 bin/nexus $(prefix)/bin/nexus

clean:
	 rm -r -f bin && rm -r -f objects
