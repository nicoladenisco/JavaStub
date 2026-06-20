#include "framework.h"
#include "JavaInfo.h"

JavaInfo::JavaInfo()
	: bit(0)
{
}

JavaInfo::JavaInfo(const QString& s, int __bit)
	: path(s), bit(__bit)
{
	std::wregex oldStyle(L"^.+\\\\(jdk|jre)([0-9]+)\\.([0-9]+)\\.([0-9]+)_([0-9]+)$", std::regex_constants::icase);
	std::wregex newStyle(L"^.+\\\\(jdk|jre)-([0-9.]+).*$", std::regex_constants::icase);

	std::wcmatch cm;
	if (std::regex_match(s.c_str(), cm, oldStyle))
	{
		if (cm.size() >= 6)
		{
			tipo = cm[1];
			v.push_back(_wtoi(QString(cm[2]).c_str()));
			v.push_back(_wtoi(QString(cm[3]).c_str()));
			v.push_back(_wtoi(QString(cm[4]).c_str()));
			v.push_back(_wtoi(QString(cm[5]).c_str()));
			return;
		}
	}

	if (std::regex_match(s.c_str(), cm, newStyle))
	{
		if (cm.size() >= 3)
		{
			tipo = cm[1];
			parseVersionParts(QString(cm[2]));
		}
	}
}

int JavaInfo::compareVersion(const JavaInfo& ji) const
{
	size_t num = v.size() > ji.v.size() ? v.size() : ji.v.size();

	for (size_t i = 0; i < num; i++)
	{
		int k1 = i < v.size() ? v[i] : 0;
		int k2 = i < ji.v.size() ? ji.v[i] : 0;

		if (k1 > k2)
			return 1;

		if (k1 < k2)
			return -1;
	}

	return 0;
}

void JavaInfo::parseVersionParts(const QString& version, const QString& separator)
{
	std::wregex rgx(separator);
	std::wsregex_token_iterator iter(version.begin(), version.end(), rgx, -1);
	std::wsregex_token_iterator end;

	while (iter != end)
	{
		v.push_back(_wtoi(QString(*iter).c_str()));
		++iter;
	}
}

