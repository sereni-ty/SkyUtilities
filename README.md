# SkyUtilities

### What's this about?

SkyUtilities is a SKSE plugin targeted to Skyrim mod authors. It adds features to Papyrus which yet don't exist and aren't available anywhere else. 
This project is a way for me to get an understanding of SKSE and the the games internal funtionalites since I'd like to start working on Skyrim SE and Fallout 4 too, as soon as there is a release of their script extensions. I'll try to implement every feature written for Oldrim into SSE as to not make the plugin irrelevant in the future if SSE actually leaves Oldrim behind (modding-scene wise).

Right now there is not very much that actually adds to Papyrus except for a basic HTTP requests (WIP) which were an open request from a LL user I picked up and started working on. (See **Future**).

### Future (or.. *I need features!*)

Planned:
+ Basic HTTP requests
  + Response acquisition
  + Callbacks/Events on state changes

+ Writing examples

Thinking about:
+ Storage of data into separate savegame (Need more information to decide on that one)

**If you need something for your mod which might work as an utility for other mods too, contact me here or in the respective LL support thread (favored).**

### Requirements

## Libraries

+ [SKSE][site_skse] (≥ *v1.9.32.0*)
+ [libCURL][site_curl] (≥ *7.54.0*)
+ [boost][site_boost] (≥ *1.63.0*)

## Compiler

A compiler with partial C++17 feature implementation (std::any, nested namespaces, *try_emplace*, ..)

[site_skse]: http://skse.silverlock.org
[site_curl]: https://curl.haxx.se/
[site_boost]: http://www.boost.org
