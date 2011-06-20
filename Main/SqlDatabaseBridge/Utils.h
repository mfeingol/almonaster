#pragma once

#include "Osal/Variant.h"

using namespace System::Data;

static const char* IdColumnName = "Id";

System::Object^ Convert(const Variant& v);
void Convert(System::Object^ object, Variant* pv);

SqlDbType Convert(VariantType type);

unsigned int* ConvertIdsToKeys(System::Collections::Generic::IEnumerable<int64>^ ids, unsigned int* piCount);

void Trace(System::String^ fmt, ... array<System::Object^>^ params);