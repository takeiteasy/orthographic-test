default:
	$(CC) -Ideps -Ideps/ez -Ideps/cwcGL/src -DCWCGL_VERSION=3000 -DGL_SILENCE_DEPRECATION  $(CFLAGS) src/*.c deps/cwcGL/src/cwcgl.c -framework GLUT -o build/tbce

.PHONY: default
