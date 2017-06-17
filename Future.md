# Thoughts

+ Papyrus load minimizer:
  + (Check): Amount of (active) scripts
  + (Check): Event (Custom or native) registration frequency
  + (Check): Pending events (load)
  + (Action): Rescheduling or, if mod is known (Configuration?), removal of event

+ Net: Retrieval of mod versions from Nexus and Loverslab. ~Not sure if achieveable. Could be valuable though.~ Event: [Net Papyrus Script]. ~Halfway~ implemented. See: [Mod Information Retrieval #2].

+ ~Net: Now that mod information is retrievable (minus nexus adult content)~
  + ~(Check): Plugin list~
  + ~(Check): Get each plugins name (PluginManager?)~
  + ~(Action): Automatically (on startup) retrieve every mods version and the version~
  Nevermind. It wouldn't be precise enough and most likely only work on mods with a unique enough name. 

[Net Papyrus Script]: https://github.com/sereni-ty/SkyUtilities/blob/master/SkyUtilities/Papyrus%20Exports/SKUNet.psc#L49-L69
[Mod Information Retrieval #1]: https://github.com/sereni-ty/SkyUtilities/commit/7c133bd91f9874f7def6baf63b41434e2367cdb5
[Mod Information Retrieval #2]: https://github.com/sereni-ty/SkyUtilities/blob/master/Examples.md


+ MCM:
  + Configuration 
  + Monitoring
