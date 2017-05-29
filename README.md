# SkyUtilities

### What's this about?

SkyUtilities is a SKSE plugin targeted to Skyrim mod authors. It adds features to Papyrus which yet don't exist and aren't available anywhere else. 
This project is a way for me to get an understanding of SKSE and the the games internal funtionalites since I'd like to start working on Skyrim SE and Fallout 4 too, as soon as there is a release of their script extensions. I'll try to implement every feature written for Oldrim into SSE as to not make the plugin irrelevant in the future if SSE actually leaves Oldrim behind (modding-scene wise).

There aren't much features at the moment but I'm getting there. Right now there are more or less fully working basic HTTP requests (bot GET and POST), URL en/decoding and the possibility to retrieve mod information from Nexus or LoversLab (via mod id). 

Requests do not block. Responses are distributed via events. Events and requests alike are serializeable which basically means that open requests are saved into the SKSE save game and continued (as it should) after loading the save game.

For more information see **Future**.

### Future (or.. *I need features!*)

Planned:
+ More [Examples][repo_examples_link]

Thinking about:
+ See [Future][repo_future_link]

**If you need something for your mod which might work as an utility for other mods too, contact me here or in the respective LL support thread (favored).**

### Requirements

#### Libraries

+ [SKSE][site_skse] (≥ *v1.9.32.0*)
+ [libCURL][site_curl] (≥ *7.54.0*)
+ [boost][site_boost] (≥ *1.63.0*)

#### Compiler

A compiler with partial C++17 feature implementation (std::any, nested namespaces, *try_emplace*, ..)

[site_skse]: http://skse.silverlock.org
[site_curl]: https://curl.haxx.se/
[site_boost]: http://www.boost.org
[repo_future_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/Future.md#thoughts
[repo_examples_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/Examples.md
