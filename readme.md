
# UnderDark v0.2
## Bloody Cactus aka @stu on github



## Contents
* [The Start](#the-start)
* [The Game Story](#the-game-story)
* [Versions](#versions)

---


## The Start

How did we get here? a DOS roguelike in 2023?

The story of the Underdark starts waaaay back in the late 90's, it was a first
roguelike that I made public, it was bad and went unfinished. However, this
version of Underdark is unrelated, it is completely rewritten from scratch...
It was my goal during the Twitch DOSember/2022 marathon to write a DOS game,
and here we are, the 4th of March 2023... so a few months after DOSember
finished. Obviously I didn't reach my goal of completing it by the end of
December, but 95% of it was done and I just needed two more months (of not
working on it very much) to clean it up.

I'm quite proud of how much work got done over the course of one month, but it
was only close to being completed, it took another month to really take off
the rough edges and make it full workable, making February a busy month as I
mostly ignored it during January.

So here we are, beginning of March and I'm drawing a line under it. I could
keep on going, tinkering with it, and it needs a few things added (like
fleshing out more food/potions). I'd also like to rescale item balance drops,
which it tries to do now but its not very good, its not granular enough I
think.

The project also ballooned out of control somewhat. The initial goal was a
simple roguelike that ran on an XT, 4.77mhz should be enough! But alas, it was
not. I got too fancy, too technical. Sure the C compiler could output for an
XT but it ran like molasses, and even on a 286 it ran like a dog.

I was really bummed, as a lot of my design decisions had been built to run
with low memory allocation, 16 bit integers etc. it had to fit into 640kb,
deal with maps of under 64kb etc to make life easier. I also think I ran into
a Watcom C compiler bug, it was corrupting memory somewhere, but none of the
other versions (win 32/64, Linux 64) were running into, and extensive memory
testing found anything.. it was only DOS real mode, despite that it was only
running in under 256kb of memory and had plenty of base memory to spare,
something was wrong.

It worked but was too dang slow. I was doing too much under the hood, tracking
too much state per move, iterating too many lists, casting too many rays for
field of vision etc.

So despite making a DOS version (for protected mode), I feel I didn’t hit my
original goal.



## The Game Story
The darkness below, its deep and dank, and you really want to go down there.
Fortune awaits, and lots of it for the brave or foolish. The greatest
treasures lie deeper than you know you can go. But you will try, and fail, the
allure bringing you back, again and again like a tortured soul looking for
redemption.

Treasure is the greediest of prizes. How much blood will you shed to take
yours? You have to make it to the lowest level to find your prize, the crown
of darkness and escape alive!


## Versions
* DOS
Requires a 386+ with 2mb or more of memory, it runs in protected mode.

* Windows
There is a 32bit SDL2 build tested on Windows XP
There is a 64bit SDL2 build tested on Windows 10 + 11

* Mac
I dont have one to test one or do a cross build

* Linux
Works with SDL2

