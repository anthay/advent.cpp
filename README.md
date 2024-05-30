# Adventure

Will Crowther created Colossal Cave Adventure in 1976 in FORTRAN IV on a PDP-10. This is a faithful recreation in C++. Here are his [code](https://github.com/anthay/Adventure/blob/main/doc/advf4.77-03-31.txt) and [data](https://github.com/anthay/Adventure/blob/main/doc/advdat.77-03-31.txt) files.

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

---

### A map of Colossal Cave

I made a map of the cave to help me test it. Obviously, it contains spoilers. (I used Flowchart Designer 3 by Guangjian Zhang.)

![A flowchart-style map of all the locations in Will Crowther's Colossal Cave Adventure.](https://github.com/anthay/Adventure/blob/master/doc/ColossalCaveMap.png)


