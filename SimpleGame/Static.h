#pragma once

#include "./Config.h"
#include "./Single.h"

namespace Suite { namespace Login { namespace Protocol {
namespace NTable {

class Static
:	public					Single<Static>
{
public:
	Config					Config;

};
static inline				Static *
STATIC()					{ return Static::Instance(); }

} /* NTable */
} /* Protocol */ } /* Login */ } /* Suite */
