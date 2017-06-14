# Examples 

## SteamAPI

### Logging the Steam User Profile ID of the currently logged in user

```papyrus
event OnInit()
  Debug.Trace("Steam Profile ID: " + SKUSteam.CurrentSteamUserProfileID())
endEvent
```

## Net

### Mod Information Retrieval (using HTTP GET Requests)

```papyrus
event OnInit()
  SKUNet.GetLLabModInfo(self, "150") ; SexLab Framework
  SKUNet.GetNexusModInfo(self, "75861") ; (Book of UUNP) - Adult
  SKUNet.GetNexusModInfo(self, "68000") ; (XPMSE) - Non Adult
endEvent

event OnModInfoRetrieval(int request_id, bool request_failed, string mod_name, string mod_version, string mod_last_updated, string mod_added, string mod_downloads, string mod_views)
  MiscUtil.PrintConsole("Mod Information: " + mod_name + " (" + mod_version + ") - last updated: " + mod_last_updated + ", added: " + mod_added + ", downloads " + mod_downloads + ", views: " + mod_views)
endEvent
```

###### Note: Script has to be some kind of Form to work.

Result: 

![Net: Mod Retrieval Console Output][net-mod-ret-con-out]

Regarding the _ADULT-ONLY_ output, see [SKUNet Papyrus Source][repo_net_psc_link]

[net-mod-ret-con-out]: http://i.imgur.com/quLfmcO.png
[repo_net_psc_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/SkyUtilities/Papyrus%20Exports/SKUNet.psc#L23
