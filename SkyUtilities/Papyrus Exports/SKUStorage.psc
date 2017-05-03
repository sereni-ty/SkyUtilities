scriptname SKUStorage hidden

; 
; These functions load single values stored in a sus savegame. 
; 
; @param	string	name	Value name. You may want to use a path (like 'modname.options.option')
; @param	mixed	default	Default value if not found
; @param	bool	store	If the value by the given name is not found in a savegame it'll be set with the default value. By default set to True
;
; @return	mixed	Either loaded value or default value is being returned
;
bool function LoadBoolean(string name, bool default_value = false) native global
string function LoadString(string name, string default_value = "") native global
int function LoadInteger(string name, int default_value = 0) native global
float function LoadFloat(string name, float default_value = 0.0) native global

; 
; These functions load arrays stored in a sus savegame. 
; 
; @param	string	name	Array name. You may want to use a path (like 'modname.options.option')
; @param	array	specific_default_values This default array will be taken if the variable couldn't be found in the save game, if you don't change default_size. Changing default_size to a higher value will fill up the remaining fields unspecified by the specific_default_values array up with the non_specific_fill_value parameter value. If default_size is lower than the specific_default_values array size, only a part of these values will make it into the result array.
; @param	bool	default_size	Size of the result array if variable couldn't be found in the save game
; @param	bool	non_specific_fill_value	Value used to fill up the remaining fields unspecified by specific_default_values (if default_size is higher than the size of specific_default_values)
;
; @return	array	Either loaded array or array with default values is being returned
;
;bool[] function LoadBooleanArray(string name, bool[] specific_default_values = none, int default_size = -1, bool non_specific_fill_value = false) native global
;function LoadStringArray(string name, string default = "", bool store = true) native global
;int[] function LoadIntegerArray(string name, int default = 0, bool store = true) native global
;float[] function LoadFloatArray(string name, float default = 0.0, bool store = true) native global 

; 
; These functions store values into a sus savegame. 
;
; @param	string	name	Value name 
; @param	mixed	value	Value to store
;
function StoreBoolean(string name, bool value) native global
function StoreString(string name, string value) native global
function StoreInteger(string name, int value) native global
function StoreFloat(string name, float value) native global

; 
; These functions store arrays into a sus savegame. 
;
; @param	string	name	Array name
; @param	array	values Array to store
;
;function StoreBooleanArray(string name, bool[] values) native global
;function StoreStringArray(string name, string[] values) native global
;function StoreIntegerArray(string name, int[] values) native global
;function StoreFloatArray(string name, float[] values) native global


;
; Returns an error code if there has an error occurred while loading / storing data
;
; 0: No error
; 1: Variable / Path unknown, couldn't load from name/path
; 2: No Savegame was loaded
; 
; @return	int	Error code
;
int function GetError() native global
