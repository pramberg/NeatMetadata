# Blueprint Property Extensions
Extension to blueprint properties to allow various metadata to be used that is otherwise only available in C++.

## Supported Metadata
### Edit Conditions
* `EditCondition`: Disable editing of the variable based on some condition.
* `EditConditionHides`: Hides a property if it doesn't match the above condition.
* `InlineEditConditionToggle`: Display the checkbox used as edit condition on the same row as the property.

### General / Misc
* `TitleProperty`: [`Array`] - determines the format of the header on each array element.
* `Categories`: [`GameplayTag`] - The root gameplay tag that can be selected.
* `AssetBundles`: [`SoftObjectReference`, `SoftClassReference`] - The root gameplay tag that can be selected.
* `AllowedTypes`: [`PrimaryAssetId`] - Determines what `PrimaryAssetType`s are allowed to be selected.
* `HideAlphaChannel`:  [`Color`, `LinearColor`] - Don't show the alpha channel in the widget.
* `GetOptions`: [`String`/`Name`] - Create a dropdown of possible values.

### Numeric Properties
* `Units`: Choose what unit a property represents. (Meters, centimeters, degrees, bytes, etc.)
* `ForceUnits`: Force the property widget to display a certain unit, regardless of user preferences.

### Directory Path
* `RelativePath`
* `ContentDir`
* `RelativeToGameContentDir`
* `LongPackageName`

### File Path
* `RelativeToGameDir`
* `FilePathFilter`

### Curves
* `XAxisName`: The name to display on the X axis of a runtime curve.
* `YAxisName`: The name to display on the Y axis of a runtime curve.