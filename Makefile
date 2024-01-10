default:
	$(CC) -I/opt/homebrew/include -L/opt/homebrew/lib -Ideps -Ideps/ez -Ideps/cwcGL/src -DCWCGL_VERSION=3000 -DGL_SILENCE_DEPRECATION -fenable-matrix $(CFLAGS) src/*.c deps/cwcGL/src/cwcgl.c -lglfw -o build/tbce

.PHONY: default
