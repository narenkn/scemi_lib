#include "scemi_pipes.cc"
#include "scemi.h"

int version = "$Rev: 5 $";

void
tba_profile (void)
{}

int
SceMi::Version(const char* versionString)
{
	uint32_t major, minor, patch;
	sscanf(versionString, "%u.%u.%u", &major, &minor, &patch);
	if (major >= 2)
		return (major<<24) | (minor<<16) | patch;
	return -1;
}

SceMi*
Init(int version, const SceMiParameters * parameters, SceMiEC* ec = 0)
{
	scemi_initialize();
}

void
RegisterErrorHandler(SceMiErrorHandler errorHandler, void *context)
{
}

