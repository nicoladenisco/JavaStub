#pragma once

#include "JavaStub.h"

class JavaInfo
{
public:
	JavaInfo();
	JavaInfo(const QString& s, int bit);
	JavaInfo(const JavaInfo& copy);
	virtual ~JavaInfo();

	inline int compare(const JavaInfo& ji) const {
		if (ji.v1 != v1)
			return ji.v1 - v1;
		if (ji.v2 != v2)
			return ji.v2 - v2;
		if (ji.v2 != v2)
			return ji.v3 - v3;
		if (ji.v4 != v4)
			return ji.v4 - v4;
		return ji.path.compare(path);
	}

	inline int operator < (const JavaInfo& ji) const {
		return compare(ji) < 0;
	}

public:
	QString path, tipo;
	int v1, v2, v3, v4, bit;
};

