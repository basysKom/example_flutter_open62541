# This is only used for testing witout dart

lib:
	g++ -c waterpump_ffi_lib.cpp \
	-fPIC \
	-Isdk/runtime \
	-Iopen62541/build/install_dir/include

	g++ -shared -o libwaterpump_ffi_lib.so \
	waterpump_ffi_lib.o \
	-fPIC \
	-Isdk/runtime \
	-Iopen62541/build/install_dir/include

test: lib
	g++ test.cpp -o test_binary \
	-fpermissive $(PWD)/sdk/runtime/include/dart_api_dl.c \
	-Isdk/runtime \
	-Iopen62541/build/install_dir/include \
	-Wl,-rpath=$(PWD) \
	-L$(PWD) -lwaterpump_ffi_lib \
	-Wl,-rpath=$(PWD)/open62541/build/install_dir/lib \
	-L$(PWD)/open62541/build/install_dir/lib -lopen62541 \
	-lpthread

clean:
	rm -f waterpump_ffi_lib.o libwaterpump_ffi_lib.so test_binary

clean_all: clean
	rm -rf open62541 sdk

# Using g++ will mangle symbol names so they look like: _Z21StartBackgroundThreadv
# but we need StartBackgroundThread, so we use the EXTERN_C (see waterpump_ffi_lib.h)
show_symbols:
	# This has to show StartBackgroundThread
	nm -gD libwaterpump_ffi_lib.so | grep StartBackgroundThread
