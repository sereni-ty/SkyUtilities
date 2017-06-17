# Examples 

## Configuration File 
(as of https://github.com/sereni-ty/SkyUtilities/commit/c0424877aa09038f27f62e38aede408d9cf07bee)

```json
{
    "Log": {
        "Level": "verbose"
    },
    "Interface": {
        "All": {
            "Processing": {
                "TimeLimit": "500"
            }
        },
        "Net": {
            "Requests": {
                "DefaultTimeout": "2500",
                "HTTP": {
                    "ResponseSizeLimit": "262144"
                }
            }
        }
    },
    "Papyrus": {
        "InterfaceCalls": {
            "AllowedPerTimeLimit": "100",
            "AllowedTransgressions": "3",
            "AllowanceTimeLimit": "1000"
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
  string profile_id = SKUSteam.GetSteamUserProfileID()
  string profile_name = SKUSteam.GetSteamUserProfileName()

  if profile_id != ""
    Debug.Trace("Steam Profile ID: " + profile_id)
  endif
  
  if profile_name != ""
    Debug.Trace("Steam Profile Name: " + profile_name)
  endif
endEvent
```

### Cycling through the Achievements

```papyrus
Debug.Trace("Cycling through Achievements: ")

int current_achievement_id = SKUSteam.GetNextAchievementID(-1)

while current_achievement_id >= 0
  string name = SKUSteam.GetAchievementName(current_achievement_id)
  string desc = SKUSteam.GetAchievementDescription(name)
  bool status = SKUSteam.GetAchievementStatus(name)

  string status_str = ""

  if status == true
    status_str = " (done)"
  endIf

  Debug.Trace("Achievement #"+ current_achievement_id + " '" + name + "': " + desc + status_str)

  current_achievement_id = SKUSteam.GetNextAchievementID(current_achievement_id)
endWhile
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

## StringUtil

### Simple regular expression example

```papyrus
string test_str = "This is an example"
  string pattern = "([^\\s$]{1,}(?:[\\s]{0,}))*"

  string[] match_result = SKUStringUtil.RegexMatch(pattern, test_str)
  string[] search_result = SKUStringUtil.RegexMatch(pattern, test_str)

  int i = 0

  Debug.Trace("RegEx: /" + pattern + "/ matching '" + test_str + "': ")

  if match_result.Length == 0
    Debug.Trace("RegEx: No matches.")
  else
    while i < match_result.Length
      Debug.Trace("Match #" + i + ": " + match_result[i])
      i += 1
    endWhile
  endif

  Debug.Trace("RegEx: /" + pattern + "/ searching '" + test_str + "': ")

  if search_result.Length == 0
    Debug.Trace("RegEx: No matches.")
  else
    i = 0
    while i < search_result.Length
      Debug.Trace("Match #" + i + ": " + search_result[i])
      i += 1
    endWhile
  endIf

  pattern = "an example"
  string replacement = "madness"
  Debug.Trace("RegEx: /" + pattern + "/ replacing '" + test_str + "': " + SKUStringUtil.RegexReplace(pattern, replacement, test_str))
```

[net-mod-ret-con-out]: http://i.imgur.com/quLfmcO.png
[repo_net_psc_link]: https://github.com/sereni-ty/SkyUtilities/blob/master/SkyUtilities/Papyrus%20Exports/SKUNet.psc#L23
