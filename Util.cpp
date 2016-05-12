#include "stdafx.h"
#include <stdexcept>
//#include <winerror.h>
#include <windows.h>
#include <propvarutil.h>
#include "Util.h"


void SetBooleanPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, BOOL value)
{
  PROPVARIANT var;
  HRESULT hr;

  if (!propertyStore) throw std::exception("Null parameter: propertyStore");

  if (SUCCEEDED(hr = InitPropVariantFromBoolean(value, &var)))
  {
    hr = propertyStore->SetValue(key, var);
  }

  PropVariantClear(&var);

  if (FAILED(hr))
    throw std::exception("Unable to set PropertyStore value");
}



void SetUint32PropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, UINT32 value)
{
  PROPVARIANT var;
  HRESULT hr;

  if (!propertyStore) throw std::exception("Null parameter: propertyStore");

  if (SUCCEEDED(hr = InitPropVariantFromUInt32(value, &var)))
  {
    hr = propertyStore->SetValue(key, var);
  }

  PropVariantClear(&var);

  if (FAILED(hr))
    throw std::exception("Unable to set PropertyStore value");
}



void SetInt32PropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, INT32 value)
{
  PROPVARIANT var;
  HRESULT hr;

  if (!propertyStore) throw std::exception("Null parameter: propertyStore");

  if (SUCCEEDED(hr = InitPropVariantFromInt32(value, &var)))
  {
    hr = propertyStore->SetValue(key, var);
  }

  PropVariantClear(&var);

  if (FAILED(hr))
    throw std::exception("Unable to set PropertyStore value");
}



void SetPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key, PROPVARIANT& value)
{
  if (!propertyStore) throw std::exception("Null parameter: propertyStore");

  HRESULT hr = propertyStore->SetValue(key, value);

  if (FAILED(hr))
    throw std::exception("Unable to set PropertyStore value");
}


PROPVARIANT GetPropertyStoreValue(IPropertyStore* propertyStore, const PROPERTYKEY& key)
{
  PROPVARIANT var;

  PropVariantInit(&var);

  if (!SUCCEEDED(propertyStore->GetValue(key, &var)))
  {
    throw std::exception("Unable to read PropertyStore value");
  }

  return var;
}
