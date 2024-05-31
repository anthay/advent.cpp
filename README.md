# Adventure

Will Crowther created Colossal Cave Adventure in 1976 in FORTRAN IV on a PDP-10. This is a faithful recreation in C++. Here are his [code](https://github.com/anthay/Adventure/blob/main/doc/advf4.77-03-31.txt) and [data](https://github.com/anthay/Adventure/blob/main/doc/advdat.77-03-31.txt) files.

Here is an example play...

```text
-----------------------------------------------------------------
     Will Crowther's original 1976 "Colossal Cave Adventure"
               A faithful reimplementation in C++
         by Anthony Hay, 2024  (CC0 1.0) Public Domain
              https://github.com/anthay/Adventure
-----------------------------------------------------------------
To quit hit Ctrl-C

PAUSE: INIT DONE
TO RESUME EXECUTION, TYPE: GO
ANY OTHER INPUT WILL TERMINATE THE PROGRAM.
go

WELCOME TO ADVENTURE!!  WOULD YOU LIKE INSTRUCTIONS?   

yes
SOMEWHERE NEARBY IS COLOSSAL CAVE, WHERE OTHERS HAVE FOUND  
FORTUNES IN TREASURE AND GOLD, THOUGH IT IS RUMORED    
THAT SOME WHO ENTER ARE NEVER SEEN AGAIN. MAGIC IS SAID
TO WORK IN THE CAVE.  I WILL BE YOUR EYES AND HANDS. DIRECT 
ME WITH COMMANDS OF 1 OR 2 WORDS.  
(ERRORS, SUGGESTIONS, COMPLAINTS TO CROWTHER)
(IF STUCK TYPE HELP FOR SOME HINTS)

YOU ARE STANDING AT THE END OF A ROAD BEFORE A SMALL BRICK  
BUILDING . AROUND YOU IS A FOREST. A SMALL   
STREAM FLOWS OUT OF THE BUILDING AND DOWN A GULLY.

enter building
YOU ARE INSIDE A BUILDING, A WELL HOUSE FOR A LARGE SPRING. 

THERE ARE SOME KEYS ON THE GROUND HERE. 

THERE IS A SHINY BRASS LAMP NEARBY.

THERE IS FOOD HERE. 

THERE IS A BOTTLE OF WATER HERE.   

take keys
OK   

leave building
YOU'RE AT END OF ROAD AGAIN.  

follow stream
YOU ARE IN A VALLEY IN THE FOREST BESIDE A STREAM TUMBLING  
ALONG A ROCKY BED.  

south
AT YOUR FEET ALL THE WATER OF THE STREAM SPLASHES INTO A    
2 INCH SLIT IN THE ROCK. DOWNSTREAM THE STREAMBED IS BARE ROCK.  

south
YOU ARE IN A 20 FOOT DEPRESSION FLOORED WITH BARE DIRT. SET INTO 
THE DIRT IS A STRONG STEEL GRATE MOUNTED IN CONCRETE. A DRY 
STREAMBED LEADS INTO THE DEPRESSION.    

THE GRATE IS LOCKED 

unlock grate
THE GRATE IS NOW UNLOCKED.    

go down
YOU ARE IN A SMALL CHAMBER BENEATH A 3X3 STEEL GRATE TO THE 
SURFACE. A LOW CRAWL OVER COBBLES LEADS INWARD TO THE WEST. 

THE GRATE IS OPEN.  

crawl in
YOU ARE CRAWLING OVER COBBLES IN A LOW PASSAGE. THERE IS A  
DIM LIGHT AT THE EAST END OF THE PASSAGE.    

THERE IS A SMALL WICKER CAGE DISCARDED NEARBY.    

go west
YOU ARE IN A DEBRIS ROOM, FILLED WITH STUFF WASHED IN FROM  
THE SURFACE. A LOW WIDE PASSAGE WITH COBBLES BECOMES   
PLUGGED WITH MUD AND DEBRIS HERE,BUT AN AWKWARD CANYON 
LEADS UPWARD AND WEST.   
A NOTE ON THE WALL SAYS 'MAGIC WORD XYZZY'.  

IT IS NOW PITCH BLACK. IF YOU PROCEED YOU WILL LIKELY  
FALL INTO A PIT.    

A THREE FOOT BLACK ROD WITH A RUSTY STAR ON AN END LIES NEARBY   

take rod
OK   

go back
YOU FELL INTO A PIT AND BROKE EVERY BONE IN YOUR BODY! 

PAUSE: GAME IS OVER
TO RESUME EXECUTION, TYPE: GO
ANY OTHER INPUT WILL TERMINATE THE PROGRAM.
```

### A map of Colossal Cave

I made a [map](https://github.com/anthay/Adventure/blob/master/doc/ColossalCaveMap.png) of the cave to help me test it. Obviously, it contains spoilers. (I used Flowchart Designer 3 by Guangjian Zhang.)

---

### To build and run Adventure

Note that the whole of Adventure is in the one file [advf4_77-03-31.cpp](https://github.com/anthay/Adventure/blob/master/src/advf4_77-03-31.cpp).

POSIX (e.g. macOS) (I used Apple clang version 15.0.0 that came with Xcode):

```text
clang++ -std=c++20 -pedantic -o advent advf4_77-03-31.cpp
./advent
```

Windows (I used Microsoft Visual Studio 2019 Community Edition Command Prompt):

```text
cl /EHsc /W4 /std:c++20 /Fe:advent advf4_77-03-31.cpp
advent
```

