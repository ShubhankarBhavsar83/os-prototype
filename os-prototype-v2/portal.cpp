#include "Portal.h"

// Static member initialization
std::map<std::string, std::string> Portal::levelTransitions;
bool Portal::transitionsInitialized = false;