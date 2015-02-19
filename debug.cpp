#include "debug.h"

void Debug::initialize()
{
    qSetMessagePattern("%{file}(%{line}): %{message}");
}


