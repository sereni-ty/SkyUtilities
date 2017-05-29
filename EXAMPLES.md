# Examples 

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

[net-mod-ret-con-out]: http://i.imgur.com/quLfmcO.png
