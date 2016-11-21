#pragma once

#define _COMMA		","

inline int urand() {
	int i1, i2, i3;
	i1 = rand() & 0x0fff;
	i2 = (rand() & 0x0fff) << 12;
	i3 = (rand() & 0x007f) << 24;
	return (i1 | i2 | i3);
}

inline int urandom(int nummax) {
	return (urand() * GetTickCount() % nummax);
}