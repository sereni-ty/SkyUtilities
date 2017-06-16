scriptname SKUSteam hidden

; =========================================================
; FUNCTIONS (STATIC)
; =========================================================
;

;
; Get profile ID of currenty logged in steam user
;
; @return	string  Profile ID or empty string (if SteamAPI was disabled or something went wrong)
;
string function GetSteamUserProfileID() global native

;
; Get profile name of currently logged in steam user
;
; @return	string  Profile name or empty string (if SteamAPI was disabled or something went wrong)
;
string function GetSteamUserProfileName() global native

;
; Cycles through the IDs. Start with an invalid value (everything <0).
; You need this function if you don't know the achievement names and want to cycle through them.
;
; @param	int	Previous achievement ID
; @return	int  Next valid achievement ID or an invalid value (<0) if there is no next achievement or the API was disabled
;
int function GetNextAchievementID(int previous_id) global native

;
; Returns the name of an achievement
;
; @param	int	Achievement ID
; @return	string  Returns the achievement name if the ID correct or an empty string if it's not or the API was disabled
;
string function GetAchievementName(int achievement_id) global native

;
; Get the status of an achievement
;
; @param	string	Achievement name
; @return	bool	Status of achievement or false on invalid achievement name or if the API was disabled
;
bool function GetAchievementStatus(string achievement_name) global native

;
; Get the description of an achievement
;
; @param	string	Achievement name
; @return	string  Achievement description or empty string on invalid achievement name or if the API is disabled
;
string function GetAchievementDescription(string achievement_name) global native