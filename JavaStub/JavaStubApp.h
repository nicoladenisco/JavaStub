#pragma once

#include "rapidxml.hpp"
#include "JavaStub.h"
#include "JavaInfo.h"

typedef rapidxml::xml_document<wchar_t> XmlDoc;
typedef rapidxml::xml_node<wchar_t> XmlNode;

class QStringList : public  std::vector<QString>
{
public:
	
	inline QStringList& append(const WCHAR* str) {
		push_back(QString(str));
		return *this;
	}
	inline QStringList& append(const QString& str) {
		push_back(str);
		return *this;
	}
	inline QStringList& append(const QStringList& list) {
		insert(end(), list.begin(), list.end());
		return *this;
	}
};

class JavaStubApp
{
public:
	JavaStubApp();
	JavaStubApp(QString stubLoc, QString stubName);
	virtual ~JavaStubApp();

public:
	enum RunEnv {
		Indefinito, Interno, InternoWin32, Esterno
	};

	void parseCommandLine();
	bool scanForUpdate();
	void scanForJavaVM();
	void scanForJavaVM(QString startPath);
	bool searchForFile(QString beginPath, QString fileName, QString &filePath);
	QString scanForAcrobat(QString startPath);
	QString scanForFirefox(QString startPath);
	QString scanForDicomdir(QString startPath);

	bool logInfo(int level, QString toLog);
	bool logInfo(int level, const WCHAR* fmt, ...);
	bool logCmdline(QString program, QStringList arguments);

	int  countFilesDir(QString originDir, bool recursive);
	int  deleteFilesDir(QString originDir, bool recursive);
	bool copyDir(QString originDir, QString destDir, bool deleteOrigin);
	void copyDir(QString originDir, QString destDir, int& count, int total, bool recursive);
	bool extractZipToDir(QString fileZip, QString extractPath, int extimatedNumFiles = 100);
	bool extractJvm();
	bool embeddedJvm();
	bool verifyToIgnore(const QString& toIgnore) const;

	bool testForFile(QString path);
	bool testForDir(QString path);
	QString getParent(QString path);
	QString getFileName(QString path);
	QString getFileNameWithoutExt(QString path);
	QStringList getListDirs(QString path);
	QStringList getListFiles(QString path);

	RunEnv testEnvironment();
	QString findJavaApp(QString dir);
	bool readInfoPlist();
	bool lanciaApplicazione();
	QStringList splitParams(QString param);
	void setTitle(QString message);
	void setMessage(QString message);
	void updateCount(int val, int total);
	void updateCount(int val);
	bool execute(QString program, QStringList argument, bool waitExit = true, QString currentDir = L"");
	bool isRunAsAdministrator();
	bool elevateNow();

	QString convertPathToCygwin(QString path);
	QString convertPathFromCygwin(QString path);

	bool startOperation();

private:
	QString appName, appIcon;
	QString stubLocation, stubName, javaHome, javaPgm, xmlInfoPath, javaAppPath, javaAppRoot;
	QString acrobat, firefox, dicomdir, logFile, dirTmp;
	std::vector<JavaInfo> jdkList;
	XmlDoc doc;
	XmlNode *javaProps;
	int verbose;
	WCHAR *bufferXmlAllFile;
	bool explicit64bit, explicit32bit;
	QString WorkingDirectory, VMOptions, MainClass, JVMVersion, Arguments, ClassPath;
};

extern JavaStubApp *ptApp;

#define LOG     ptApp->logInfo

#define JAVA_SDK_ONDISK		L"jre-8-windows-i586.exe"
#define JAVA_ZIP_ONDISK		L"jre8emb.zip"
#define EFILM_ZIP_ONDISK	L"efilm-pack.zip"

#define DEFAULT_LOG_FILE    L"c:\\temp\\javastub.log"

// funzioni di utilita
QString toLowercase(QString data);
QString toUppercase(QString data);
QString testQuote(QString in);
QStringList resplit(QString s, QString rgx_str = L"\\s+");
QString replaceAll(const QString& source, const QString& from, const QString& to);
QString describeWindowsError(QString function, int errorCode);
QString getEnvVar(QString key);

#define wcsdupa(x) \
	(wcscpy((WCHAR*)alloca((wcslen(x)+1) * sizeof(WCHAR)), (x)))

// test attributi di FindFirst/FindNext
#define IS_DIRECTORY(x) \
	(((x) & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
#define IS_DEVICE(x) \
	(((x) & FILE_ATTRIBUTE_DEVICE) == FILE_ATTRIBUTE_DEVICE)
#define IS_HIDDEN(x) \
	(((x) & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
#define IS_FILE(X) \
	(!IS_DIRECTORY(X) && !IS_DEVICE(X))
