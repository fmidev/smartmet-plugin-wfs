
REQUIRES = configpp

include $(shell echo $${PREFIX-/usr})/share/smartmet/devel/makefile.inc

TARGETS = decode_feature_id

#
# Boost 1.69

ifneq "$(wildcard /usr/include/boost169)" ""
  BOOST_INCLUDES := -I/usr/include/boost169
  BOOST_LIBS := -L/usr/lib64/boost169
endif

INCLUDES += -I../libwfs

DECODE_FEATURE_ID_LIBS := -L $(libdir)/smartmet $(BOOST_LIBS) -lsmartmet-spine -lsmartmet-macgyver -lcrypto

all: $(TARGETS)

clean:
	rm -f $(TARGETS)

decode_feature_id: decode_feature_id.cpp ../libsmartmet-plugin-wfs.a ../libwfs/FeatureID.h ../libwfs/BStream.h
	g++ $(CFLAGS) $(INCLUDES) -o $@ decode_feature_id.cpp ../libsmartmet-plugin-wfs.a $(DECODE_FEATURE_ID_LIBS)
