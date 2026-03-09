#pragma once

#include "vectorN.h"

struct Plane
{
	Vector3 pos{};
	Normal3 nor{Normal3::Up()};
};

struct Ray
{
	Vector3 pos{};
	Normal3 dir{Normal3::Forward()};
};
