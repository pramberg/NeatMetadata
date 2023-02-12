# Blueprint Property Extensions
Extension to blueprint properties to allow various metadata to be used that is otherwise only available in C++.

## Supported Metadata
* `EditCondition`: Disable editing of the variable based on some condition.
* `EditConditionHides`: Hides a property if it doesn't match the above condition.
* `Categories`: GameplayTag specific metadata that specifies the root gameplay tag that can be selected.

## Gameplay Tag
This is currently the only variable type that has any extensions.
* `Categories`: Allows you to choose the gameplay tag "Category" (think the root/base gameplay tag) that can be selected.

![image](/Documentation/Example_GameplayTag.png)
