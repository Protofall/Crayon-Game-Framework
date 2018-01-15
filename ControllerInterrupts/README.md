# Basic Maple controller test

...is what I should have called this example. This program lets you use the Dpad to move the nerstr face around the screen and if yu press A+B+Start then an interrupt is triggered that basically ends the program.

Originally I thoughts my Dpad code was plain old polling and hence I had to find an interrupt method, but due to how KOS works the Dpad code is actually getting data from KOS' normal routine in which it reads the controller inputs anyways. Before realising that I found a controller callback function that uses interrupts, however its very limitted in which it can only call one function and have no parameters. The KOS specs say its only really useful for ending your program soooo I decided to keep it just to show what it is and its one real purpose.

Also note if you are trying this in an emulator and say that its doing weird behaviour when pressing Left + Right and then also Down, just remember those combinations aren't physically possible with a normal dreamcast controller...I made that mistake too :P