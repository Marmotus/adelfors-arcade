windres ./extras/arcade_icon.rc -O coff -o ./extras/arcade_icon.res

gcc -o adelfors-arcade -Wall main.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image ./extras/arcade_icon.res

REM REMEMBER TO UPDATE "VERSION_STRING_LITERAL"!!!

pause
