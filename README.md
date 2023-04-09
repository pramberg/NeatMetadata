# <img src="Resources/Icon128.png" style="height:1.0em;" align="center"/> Neat Metadata
Editor extension to allow various metadata to be used in Blueprints that is otherwise only available in C++.

## Supported Metadata
### Edit Conditions
* `EditCondition`: Disable editing of the variable based on some condition.
* `EditConditionHides`: Hides a property if it doesn't match the above condition.
* `InlineEditConditionToggle`: Display the checkbox used as edit condition on the same row as the property.
* `HideEditConditionToggle`: Hide the inline checkbox on this property.

### General / Misc
* `TitleProperty`: [`Array`] - determines the format of the header on each array element.
* `Categories`: [`GameplayTag`] - The root gameplay tag that can be selected.
* `AssetBundles`: [`SoftObjectReference`, `SoftClassReference`] - Determines what asset bundles to put the object reference in. See [Asset Manager documentation](https://docs.unrealengine.com/5.1/en-US/asset-management-in-unreal-engine/#assetbundles) for more details.
* `AllowedTypes`: [`PrimaryAssetId`] - Determines what `PrimaryAssetType`s are allowed to be selected.
* `HideAlphaChannel`:  [`Color`, `LinearColor`] - Don't show the alpha channel in the widget.
* `GetOptions`: [`String`/`Name`] - Create a dropdown of possible values.
* `ShowOnlyInnerProperties`: Removes the parent scope around a struct property and displays all of its properties inline.

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

### Class Picker
* `AllowAbstract`
* `ShowTreeView`
* `BlueprintBaseOnly`
* `OnlyPlaceable`
* `DisallowCreateNew`
* `ShowDisplayNames`
* `HideViewOptions`
* `MustImplement`

### Array
* `NoElementDuplicate`
* `EditFixedOrder`

### Curves
* `XAxisName`: The name to display on the X axis of a runtime curve.
* `YAxisName`: The name to display on the Y axis of a runtime curve.
