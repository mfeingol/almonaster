#pragma once

#include "Osal/Variant.h"

using namespace System::Data;

static const char* IdColumnName = "Id";

System::Object^ Convert(const Variant& v);
void Convert(System::Object^ object, Variant* pv);

SqlDbType Convert(VariantType type, int size);

void Trace(System::String^ fmt, ... array<System::Object^>^ params);