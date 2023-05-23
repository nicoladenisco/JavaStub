#include "framework.h"
#include "JavaInfo.h"

JavaInfo::JavaInfo()
{
}

JavaInfo::JavaInfo(const JavaInfo& copy)
	: path(copy.path), tipo(copy.tipo),
	v1(copy.v1), v2(copy.v2), v3(copy.v3), v4(copy.v4), bit(copy.bit)
{
}

JavaInfo::JavaInfo(const QString& s, int __bit)
{
	path = s;
	bit = __bit;
	std::wregex pc2w(L"^.+\\\\([a-z]{3})([0-9]+)\\.([0-9]+)\\.([0-9]+)_([0-9]+)$", std::regex_constants::icase);

	std::wcmatch cm;
	if (!std::regex_match(s.c_str(), cm, pc2w))
		return;

	if (cm.size() < 5)
		return;

	tipo = cm[1];
	v1 = _wtoi(QString(cm[2]).c_str());
	v2 = _wtoi(QString(cm[3]).c_str());
	v3 = _wtoi(QString(cm[4]).c_str());
	v4 = _wtoi(QString(cm[5]).c_str());
}

JavaInfo::~JavaInfo()
{
}
