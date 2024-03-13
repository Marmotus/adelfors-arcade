1. Information
2. How to compile
3. How to add a game and what is needed

# Information
Ädelfors Arcade is a simple program written in C that aims to emulate an arcade experience for a Windows computer.

It detects games in its "games" folder and displays them in the user interface, from where they can be launched.

# How to compile
If you wish to compile the program yourself:

SDL2 is required. To install SDL2:

1. Install MSYS2 at https://www.msys2.org/

2. Install SDL2 by running MSYS2 and entering (without the quotation marks):
    "pacman -S mingw-w64-x86_64-SDL2"
    "pacman -S mingw-w64-x86_64-SDL2_ttf"
    "pacman -S mingw-w64-x86_64-SDL2_image"

3. For good measure, you can keep MSYS updated by running:
    "pacman -Sy"
    "pacman -Syy"
    "pacman -Syu"

To compile, you simply double-click the "compile.bat" file.

If the compilation fails, you may want to open a terminal window in the project folder and run "./compile.bat". That way, the errors will be printed out to the terminal.

If it can't find GCC, you may need to add the path to its folder to your system's "path" environment variable. If MSYS2 was installed to its default location, the path should be "C:\msys64\mingw64\bin". If you're compiling through the terminal you need to restart it in order for it to know that you've added the path now.

# How to add a game and what is needed
To add a game, create a new folder in the "games" folder and place your game contents in there. A .exe file must be in your newly created folder, NOT in a subfolder within it.

That is all that is required for the game to be displayed and function. But, if you want the game to have an icon in the arcade user interface, place a .png or .jpg image in the game folder and name it "game_icon.png" or "game_icon.jpg".

It is recommended that the icon image be as small in size as possible, preferably 200 x 200 pixels. This is to save on memory, since the icon doesn't need to be very large.