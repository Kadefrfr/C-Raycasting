This is my attempt at raycasting. Since I knew this would have some cool development stages, I decided that any time I make a significant change (anything more than changing a single value, such as the player's movement speed), I would create a new file. It's not too organized, but a big change like from 2.02 to 3.01 is due to two major changes: the colors and the player's movement. When I changed from 3.01 to 3.51, I thought that since I only added one feature (wall collision), it wasn't as significant as the previous update.

To run any of this, you have to clone this repository with: ```git clone https://github.com/Kadefrfr/C-Raycasting```

Then change to that directory with:```cd C-Raycasting```

Now you can choose which version you want to compile. For example, to compile version 3.51, use:```gcc -o raycastv-3.51 raycastv-3.51.c -lSDL2 -lm```

This will compile it. Now you want to run it with:```./raycastv-3.51```

Use the first name you typed after ```-o``` as the name of the application. The game should now run, and you can move the character with the arrow keys.
