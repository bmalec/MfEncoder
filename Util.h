#pragma once

#include <propsys.h>


void SetBooleanPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, BOOL value);
void SetUint32PropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, UINT32 value);
void SetInt32PropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, INT32 value);
void SetPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, PROPVARIANT& value);

PROPVARIANT GetPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key);



