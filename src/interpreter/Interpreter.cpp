#include "Interpreter.h"
#include <algorithm>

SContext::SContext()
{
	Reset();
}

void SContext::Reset()
{
	std::fill(V.begin(), V.end(), 0);
	I = 0;
	PC = 0;
	SP = 0;
	DT = 0;
	ST = 0;
}
