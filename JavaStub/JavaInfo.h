#pragma once

#include "JavaStub.h"

class JavaInfo
{
public:
	JavaInfo();
	JavaInfo(const QString& path, int bit);

	int compareVersion(const JavaInfo& ji) const;
	void parseVersionParts(const QString& version, const QString& separator = L"\\.");

	inline int compare(const JavaInfo& ji) const
	{
		int cmp = compareVersion(ji);
		if (cmp != 0)
			return cmp;

		return path.compare(ji.path);
	}

	inline bool operator < (const JavaInfo& ji) const
	{
		return compare(ji) < 0;
	}

public:
	QString path;
	QString tipo;
	std::vector<int> v;
	int bit;
};

