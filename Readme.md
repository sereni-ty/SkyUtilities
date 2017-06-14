# SkyUtilities

### What's this about?

SkyUtilities is a SKSE plugin targeted to Skyrim mod authors. It adds features to Papyrus which yet don't exist and aren't available anywhere else. 
This project is a way for me to get an understanding of SKSE and the the games internal funtionalites since I'd like to start working on Skyrim SE and Fallout 4 too, as soon as there is a release of their script extensions. I'll try to implement every feature written for Oldrim into SSE as to not make the plugin irrelevant in the future if SSE actually leaves Oldrim behind (modding-scene wise).

### Features 

There aren't much features at the moment but I'm getting there. 

+ (Basic) HTTP requests (both GET and POST)
+ URL en/decoding 
+ Mod information retrieval from Nexus (non-adult) or LoversLab (via mod id).
+ Steam User Profile ID acquiral

Requests do not block. Responses are distributed via events. Events and requests alike are serializeable which basically means that open requests (Requests not processed when the game is saved) are saved into the SKSE co-save and continued (as it should) after loading the save game.

### Future (or.. *I need features!*)

Planned:
+ More [Examples][repo_examples_link]

Thinking about:
+ See [Future][repo_future_link]

**If you need something for your mod which might work as an utility for other mods too, contact me here or in the respective LL support thread (favored).**

### Requirements

If you want to compile SkyUtilities on your own. 

#### Libraries 

+ [SKSE][site_skse] (≥ *v1.9.32.0*)
+ [libCURL][site_curl] (≥ *7.54.0*)
+ [boost][site_boost] (≥ *1.63.0*) (Header only)
+ [Steamworks][site_steamworks] (≥ *140*) (Header only)

#### Compiler

A compiler with partial C++17 feature implementation (std::any, nested namespaces, *try_emplace*, ..)

[site_skse]: http://skse.silverlock.org
[site_curl]: https://curl.haxx.se/
[site_boost]: http://www.boost.org
[site_steamworks]: https://partner.steamgames.com/home
[repo_future_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/Future.md#thoughts
[repo_examples_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/Examples.md
