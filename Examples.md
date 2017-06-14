# Examples 

## Configuration File 
(as of https://github.com/sereni-ty/SkyUtilities/commit/c0424877aa09038f27f62e38aede408d9cf07bee)

```json
{
    "Net": {
        "Papyrus": {
            "CallsPerTimeFrame": "100",
            "MaxTransgressions": "3",
            "TimeFrame": "1000"
        },
        "Requests": {
            "DefaultTimeout": "2500",
            "HTTP": {
                "MaxResponseSize": "262144"
            }
        }
    },
    "SteamAPI": {
        "Enabled": "true"
    }
}
```

This configuration file is automatically created on the first startup of a new game and is therefore filled with default values. SkyUtilites does not change any of these values. 
You can customize the values to your liking, though you can't go above these values as they're not only default values but also hard caps.

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
