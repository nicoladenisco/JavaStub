#include "framework.h"
#include "JavaStubApp.h"

static const WCHAR *arPaths64[] = {
	TEXT("c:\\programmi"),
	TEXT("c:\\program files"),
	NULL
};

static const WCHAR *arPaths32[] = {
	TEXT("c:\\programmi (x86)"),
	TEXT("c:\\program files (x86)"),
	NULL
};

static const WCHAR* ignoreJava[] = {
	TEXT("android"),
	NULL
};

JavaStubApp *ptApp = NULL;

// funzioni definite in JavaStub3.cpp
void fatalError(const WCHAR* errorMsg);
void updateCount(int val);
void setTitle(const WCHAR* title);
void setMessage(const WCHAR* message);


JavaStubApp::JavaStubApp()
{
	ptApp = this;
	logFile = DEFAULT_LOG_FILE;
	verbose = 3;
	explicit64bit = explicit32bit = autoScanJars = false;
	bufferXmlAllFile = NULL;

	// imposta directory temporanea
	dirTmp = L"c:\\temp";
	QString tmp = getEnvVar(L"appdata");
	if (!tmp.empty())
		dirTmp = tmp + L"\\javastub";

	// file di log all'interno della directory temporanea
	logFile = dirTmp + L"\\javastub.log";

	// crea se non fosse già presente la directory c:/temp
	::_wmkdir(dirTmp.c_str());
	// rimuovo vecchio file di log
	::_wunlink(logFile.c_str());

	LOG(5, L"Parametri argc: %d\n", __argc);
	if (__argc <= 0)
	{
		fatalError(L"Nessun parametro sulla linea di comando.");
		throw 0;
	}
}

JavaStubApp::JavaStubApp(QString stubLoc, QString stubNm)
{
	stubLocation = stubLoc;
	stubName = stubNm;
	ptApp = this;
	logFile = DEFAULT_LOG_FILE;
	verbose = 3;
	explicit64bit = explicit32bit = false;
	bufferXmlAllFile = NULL;
}

JavaStubApp::~JavaStubApp()
{
	if (bufferXmlAllFile != NULL)
		free(bufferXmlAllFile);
}

void JavaStubApp::parseCommandLine()
{
	for (int i = 0; i < __argc; i++)
		LOG(5, L"argv[%d]=[%s]\n", i, __wargv[i]);

	QString path(__wargv[0]);
	stubLocation = getParent(path);
	stubName = getFileNameWithoutExt(path);

	for (int i = 1; i < __argc; i++)
	{
		QString s = __wargv[i];

		if (s == L"--pwd")
		{
			WCHAR buffer[1024];
			GetCurrentDirectory(1024, buffer);
			stubLocation = buffer;
			continue;
		}

		if (s == L"--verbose")
		{
			if ((i + 1) < __argc)
				verbose = _wtoi(__wargv[++i]);

			continue;
		}

		if (s == L"--name")
		{
			if ((i + 1) < __argc)
				stubName = __wargv[++i];

			continue;
		}

		if (s == L"-v")
		{
			verbose++;
			continue;
		}
	}

	LOG(1, L"stubLocation=%s\nstubName=%s\n", stubLocation.c_str(), stubName.c_str());
}

int bit = 0;
bool fsortInfo(JavaInfo i, JavaInfo j) { return (i < j); }

void JavaStubApp::scanForJavaVM()
{
	if (!explicit32bit)
	{
		bit = 64;
		for (int i = 0; arPaths64[i] != NULL; i++)
		{
			if (testForDir(arPaths64[i]))
			{
				LOG(1, L"Inizio scansione directory %s ...\n", arPaths64[i]);
				scanForJavaVM(QString(arPaths64[i]));
			}
		}
	}

	if (!explicit64bit)
	{
		bit = 32;
		for (int i = 0; arPaths32[i] != NULL; i++)
		{
			if (testForDir(arPaths32[i]))
			{
				LOG(1, L"Inizio scansione directory %s ...\n", arPaths32[i]);
				scanForJavaVM(QString(arPaths32[i]));
			}
		}
	}

	// sorta le jdk in ordine alfabetico: probabilmente la più recente sara' l'ultima
	if (!jdkList.empty())
	{
		std::sort(jdkList.begin(), jdkList.end(), fsortInfo);
		const JavaInfo &ji = jdkList[0];
		javaHome = ji.path;
		LOG(1, TEXT("JAVA_HOME=") + javaHome + TEXT("\n"));
	}
}

/*
* estrae i java jdk/jre che riesce a trovare
*/
void JavaStubApp::scanForJavaVM(QString startPath)
{
	WIN32_FIND_DATA fdata;
	QString dirJava(startPath + L"\\*.*");
	HANDLE h = FindFirstFile(dirJava.c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (IS_DIRECTORY(fdata.dwFileAttributes))
		{
			QString fileName(fdata.cFileName);
			if (fileName == L"." || fileName == L"..")
				continue;

			QString d = startPath + L"\\" + fileName;
			QString testName = toLowercase(fileName);

			if (verifyToIgnore(testName))
				continue;

			if (testName.find(L"jdk-") != QString::npos ||
				testName.find(L"jre-") != QString::npos)
			{
				// memorizza per successivo sort
				jdkList.push_back(JavaInfo(d, bit));
				LOG(2, TEXT("Add jdk %s\n"), d.c_str());
			}
			else
			{
				scanForJavaVM(d);
			}
		}

	} while (FindNextFile(h, &fdata));

	FindClose(h);
}


bool JavaStubApp::verifyToIgnore(const QString& toIgnore) const
{
	for (int i = 0; ignoreJava[i] != NULL; i++)
		if (toIgnore.find(ignoreJava[i]) != QString::npos)
			return true;

	return false;
}

/*
void JavaStubApp::scanForPrograms(QString startPath)
{
	WIN32_FIND_DATA fdata;
	QString dirJava(startPath + L"\\java\\*.*");
	HANDLE h = FindFirstFile(dirJava.c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (IS_DIRECTORY(fdata.dwFileAttributes))
		{
			QString fileName = toLowercase(fdata.cFileName);
			if (fileName.find_first_of(L"jdk") != QString::npos ||
				fileName.find_first_of(L"jre") != QString::npos)
			{
				// memorizza per successivo sort
				QString d = startPath + L"\\java\\" + fdata.cFileName;
				jdkList.push_back(JavaInfo(d, bit));
				LOG(2, TEXT("Add jdk %s\n"), d.c_str());
			}
		}

	} while (FindNextFile(h, &fdata));

	FindClose(h);
}
*/

bool JavaStubApp::searchForFile(QString beginPath, QString fileName, QString &filePath)
{
	if (testForFile(beginPath + L"\\" + fileName))
	{
		filePath = beginPath + L"\\" + fileName;
		return true;
	}

	QStringList subdir = getListDirs(beginPath);
	for (QStringList::const_iterator itr = subdir.begin(); itr != subdir.end(); itr++)
	{
		QString newBeginPath(beginPath + L"\\" + (*itr));
		if (searchForFile(newBeginPath, fileName, filePath))
			return true;
	}

	return false;
}

QString JavaStubApp::scanForAcrobat(QString startPath)
{
	QString pgmPath;
	if (searchForFile(startPath, L"AcroRd32.exe", pgmPath))
		return pgmPath;

	return L"";
}

QString JavaStubApp::scanForFirefox(QString startPath)
{
	QString pgmPath;
	if (searchForFile(startPath, L"firefox.exe", pgmPath))
		return pgmPath;

	return L"";
}

QString JavaStubApp::scanForDicomdir(QString startPath)
{
	QString pgmPath;
	if (searchForFile(startPath, L"DICOMDIR", pgmPath))
		return pgmPath;

	return L"";
}

/**
* Verifica e installa aggiornamenti.
* @return vero se nessun errore rivelato
*/
bool JavaStubApp::scanForUpdate()
{
	QString appName = getFileName(javaAppPath);
	QString appUpdate = L"c:\\temp\\" + appName;

	LOG(1, L"Verifico aggiornamento con %s\n", appUpdate.c_str());
	if (!testForDir(appUpdate))
	{
		// verifica aggiornamento nella nuova posizione in appdata/javastub/update
		appUpdate = dirTmp + L"\\update\\" + appName;
		LOG(1, L"Verifico aggiornamento con %s\n", appUpdate.c_str());
	}

	if (testForDir(appUpdate))
	{
		// per installare gli aggiornamenti occorre rilanciare il programma come amministratore
		// se il rilancio va a buon fine viene sollevata una eccezione fittizia per uscire immediatamente
		if (!isRunAsAdministrator())
			if (elevateNow())
				throw 0;

		LOG(2, L"Inizio aggiornamento con %s\n", appUpdate.c_str());
		setMessage(L"Aggiornamento in corso ...");

		// trovato aggiornamento nella directory temp per l'applicazione
		copyDir(appUpdate, javaAppPath, true);

		// a scanso di equivoci se la directory di aggiornamento 
		// non è stata cancellata cerca di rinominarla in modo che 
		// allo startup successivo venga ignorata
		QString oldUpdate = appUpdate + L".old";
		deleteFilesDir(oldUpdate, true);
		::_wrename(appUpdate.c_str(), oldUpdate.c_str());
	}
	else
		LOG(1, L"Nessun aggiornamento trovato.\n");

	return true;
}

/**
* Copia il contenuto di una directory (non la directory stessa)
* all'interno di un'altra directory.
* @param originDir directory origine
* @param destDir directory destinazione
* @param deleteOrigin cancella la directory origine e il suo contenuto
* @return vero operazione riuscita
*/
bool JavaStubApp::copyDir(QString originDir, QString destDir, bool deleteOrigin)
{
	int numFiles = countFilesDir(originDir, true);

	// imposta clessidra avanzamento
	updateCount(0, numFiles);

	int count = 0;
	copyDir(originDir, destDir, count, numFiles, true);

	if (deleteOrigin)
		deleteFilesDir(originDir, true);

	return true;
}

/**
* Funzione di servizio per copia ricorsiva directory.
* @param originDir directory origine
* @param destDir directory destinazione
* @param count contatore files copiati
* @param total numero totale files da copiare (per aggiornamento clessidra)
* @param recursive copia ricorsivamente le directory contenute
* @param deleteOrigin cancella la directory origine e il suo contenuto
*/
void JavaStubApp::copyDir(QString originDir, QString destDir, int& count, int total, bool recursive)
{
	LOG(5, L"copyDir(%s, %s)\n", originDir.c_str(), destDir.c_str());

	QStringList fileList = getListFiles(originDir);
	for (QStringList::const_iterator itr = fileList.begin(); itr != fileList.end(); itr++)
	{
		QString f = originDir + L"\\" + (*itr);
		QString d = destDir + L"\\" + (*itr);
		LOG(5, L"copyDir(...) [%d] %s=>%s \n", count, f.c_str(), d.c_str());

		// aggiorna interfaccia
		updateCount(++count, total);

		if (testForFile(d))
			::_wunlink(d.c_str());

		::CopyFile(f.c_str(), d.c_str(), FALSE);
	}

	if (!recursive)
		return;

	QStringList dirList = getListDirs(originDir);
	for (QStringList::const_iterator itr = dirList.begin(); itr != dirList.end(); itr++)
	{
		QString f = originDir + L"\\" + (*itr);
		QString d = destDir + L"\\" + (*itr);

		::_wmkdir(d.c_str());
		copyDir(f, d, count, total, recursive);
	}
}

/**
* Conta i file presenti in una directory.
* @param originDir directory da contare
* @param recursive vero per ricerca ricorsiva
* @return numero totale files contenuti
*/
int JavaStubApp::countFilesDir(QString originDir, bool recursive)
{
	LOG(5, L"countFilesDir(%s)\n", originDir.c_str());
	if (!testForDir(originDir))
		return 1;

	QStringList fileList = getListFiles(originDir);
	int count = fileList.size();

	if (recursive)
	{
		QStringList dirList = getListDirs(originDir);
		for (QStringList::const_iterator itr = dirList.begin(); itr != dirList.end(); itr++)
		{
			QString d = originDir + L"\\" + (*itr);
			count += countFilesDir(d, recursive);
		}
	}

	return count;
}

/**
* Cancella una directory con tutto il suo contenuto
* @param originDir directory da cancella
* @param recursive vero per ricerca ricorsiva
* @return numero totale files processati
*/
int JavaStubApp::deleteFilesDir(QString originDir, bool recursive)
{
	LOG(5, L"deleteFilesDir(%s)\n", originDir.c_str());
	if (!testForDir(originDir))
		return 1;

	int count = 0;
	QStringList fileList = getListFiles(originDir);
	for (QStringList::const_iterator itr = fileList.begin(); itr != fileList.end(); itr++)
	{
		QString d = originDir + L"\\" + (*itr);
		if (::_wunlink(d.c_str()) == 0)
			count++;
	}

	if (recursive)
	{
		QStringList dirList = getListDirs(originDir);
		for (QStringList::const_iterator itr = dirList.begin(); itr != dirList.end(); itr++)
		{
			QString d = originDir + L"\\" + (*itr);
			count += deleteFilesDir(d, recursive);
		}
	}

	::_wrmdir(originDir.c_str());
	return count;
}

bool JavaStubApp::logInfo(int level, const WCHAR* fmt, ...)
{
	WCHAR buffer[4096];

	va_list ap;
	va_start(ap, fmt);
	vswprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	return logInfo(level, QString(buffer));
}

bool JavaStubApp::logInfo(int level, QString toLog)
{
	if (verbose && level > verbose)
		return false;

	FILE *fp;
	if (_wfopen_s(&fp, logFile.c_str(), L"a, ccs=UTF-8") == 0)
	{
		fputws(toLog.c_str(), fp);
		fclose(fp);
		return true;
	}

	return false;
}

bool JavaStubApp::logCmdline(QString program, QStringList arguments)
{
	int count = 0;
	QString cmdLine = program;

	for (QStringList::const_iterator itr = arguments.begin(); itr != arguments.end(); itr++)
	{
		cmdLine += L" " + (*itr);
		LOG(2, TEXT("Parametro %d=%s\n"), count++, itr->c_str());
	}

	LOG(1, TEXT("Command line %1\n"), cmdLine.c_str());
	return false;
}

bool JavaStubApp::extractZipToDir(QString fileZip, QString extractPath, int extimatedNumFiles /*=100*/)
{
	QString infoProg = javaAppRoot + L"\\win32\\7za.exe";
	LOG(5, L"Cerco scompattatore in %s\n", infoProg.c_str());
	if (!testForFile(infoProg))
	{
		LOG(5, L"Scompattatore non trovato: estrazione di %s non possibile.\n", fileZip.c_str());
		return false;
	}

	// 7za.exe x -ojava jre6_cipo.zip
	QString program = infoProg;
	QStringList arguments;
	arguments.append(L"x");
	arguments.append(L"-y"); // per overwrite dei files
	arguments.append(L"-o" + extractPath);
	arguments.append(fileZip);

	return execute(program, arguments);
}

bool JavaStubApp::embeddedJvm()
{
	// C:\Program Files (x86)\CaleidoRefOO\calrefoo.app\Contents\bin\win32

	QString test;

	if (!explicit64bit)
	{
		test = javaAppRoot + L"\\bin\\win32\\jre";
		LOG(5, L"Test EMBEDDED: " + test + L"\n");
		if (testForDir(test)) {
			// imposta la nuova javahome
			javaHome = test;
			LOG(1, TEXT("EMBEDDED JAVA_HOME=%s\n"), javaHome.c_str());
			jdkList.push_back(JavaInfo(test, 32));
			return true;
		}

		test = javaAppRoot + L"\\bin\\win32\\jdk";
		LOG(5, L"Test EMBEDDED: " + test + L"\n");
		if (testForDir(test)) {
			// imposta la nuova javahome
			javaHome = test;
			LOG(1, TEXT("EMBEDDED JAVA_HOME=%s\n"), javaHome.c_str());
			jdkList.push_back(JavaInfo(test, 32));
			return true;
		}
	}

	if (!explicit32bit)
	{
		test = javaAppRoot + L"\\bin\\win64\\jre";
		LOG(5, L"Test EMBEDDED: " + test + L"\n");
		if (testForDir(test)) {
			// imposta la nuova javahome
			javaHome = test;
			LOG(1, TEXT("EMBEDDED JAVA_HOME=%s\n"), javaHome.c_str());
			jdkList.push_back(JavaInfo(test, 64));
			return true;
		}

		test = javaAppRoot + L"\\bin\\win64\\jdk";
		LOG(5, L"Test EMBEDDED: " + test + L"\n");
		if (testForDir(test)) {
			// imposta la nuova javahome
			javaHome = test;
			LOG(1, TEXT("EMBEDDED JAVA_HOME=%s\n"), javaHome.c_str());
			jdkList.push_back(JavaInfo(test, 64));
			return true;
		}
	}

	return false;
}

bool JavaStubApp::extractJvm()
{
	QString zipPath = javaAppRoot + TEXT("win32/" JAVA_ZIP_ONDISK);

	// test per nessuna JVM embedded (in formato compresso)
	if (!testForFile(zipPath))
		return false;

	::_wmkdir(TEXT("c:\\temp"));
	::_wmkdir(TEXT("c:\\temp\\java"));

	QString javaPath(TEXT("c:\\temp\\java"));
	if (!testForDir(javaPath))
	{
		fatalError(TEXT("Non posso creare la directory temporanea."));
		return false;
	}

	// controlla che la jvm non sia gia' stata scompattata
	QString tmpJavaHome = TEXT("c:\\temp\\java\\jre8");
	if (!testForDir(tmpJavaHome))
	{
		LOG(1, TEXT("unpack=%s in %s\n"), zipPath.c_str(), javaPath.c_str());

		// 7za.exe x -ojava jre6_cipo.zip
		if (!extractZipToDir(zipPath, javaPath))
			return false;
	}

	// imposta la nuova javahome
	javaHome = tmpJavaHome;
	LOG(1, TEXT("JAVA_HOME=%s\n"), javaHome);

	return true;
}

JavaStubApp::RunEnv JavaStubApp::testEnvironment()
{
	LOG(1, TEXT("Locazione dell'eseguibile %s\n"), stubLocation.c_str());

	xmlInfoPath = stubLocation + TEXT("\\Info.plist");
	LOG(1, TEXT("Test per posizione interna: cerco %s\n"), xmlInfoPath.c_str());

	if (testForFile(xmlInfoPath))
	{
		// trovato file Info.plist nella stessa directory dello stub
		javaAppPath = getParent(stubLocation);
		LOG(1, TEXT("Per conferma %s deve essere la directory applicazione.\n"), javaAppPath.c_str());

		if (javaAppPath.substr(javaAppPath.length() - 4) != TEXT(".app"))
			return Indefinito;

		// lancio di applicazione java con stub all'interno della directory xxxx.app/Contents
		javaAppRoot = stubLocation;
		LOG(1, TEXT(
			"OK posizione interna soddisfatta.\n"
			"javaAppPath=%s\n"
			"javaAppRoot=%s\n"), javaAppPath.c_str(), javaAppRoot.c_str());

		return Interno;
	}

	LOG(1, TEXT("Test per posizione interna (win32)\n"));

	javaAppRoot = getParent(stubLocation);
	if (javaAppRoot == TEXT("win32"))
		javaAppRoot = getParent(javaAppRoot);

	if (testForDir(javaAppRoot))
	{
		xmlInfoPath = javaAppRoot + TEXT("\\Info.plist");
		if (testForFile(xmlInfoPath))
		{
			// trovato info.plist
			javaAppPath = getParent(javaAppRoot);

			// lancio di applicazione java con stub all'interno della directory xxxx.app/Contents/win32
			LOG(1, TEXT(
				"OK posizione interna (win32) soddisfatta.\n"
				"javaAppPath=%s\n"
				"javaAppRoot=%s\n"), javaAppPath.c_str(), javaAppRoot.c_str());

			return InternoWin32;
		}
	}

	LOG(1, TEXT("Test per posizione esterna: cerco *.app %s\n"), stubLocation.c_str());

	// cerca nella stessa directory dello stub una directory xxxx.app
	javaAppPath = findJavaApp(stubLocation);
	if (!javaAppPath.empty())
	{
		LOG(1, TEXT("Trovato un candidato in %s\n"), javaAppPath.c_str());

		// cerca il file Info.plist all'interno della directory
		javaAppRoot = javaAppPath + TEXT("\\Contents");
		xmlInfoPath = javaAppRoot + TEXT("\\Info.plist");
		LOG(1, TEXT("Cerco il file %s per conferma.\n"), xmlInfoPath.c_str());

		if (!testForFile(xmlInfoPath))
			return Indefinito;

		// lancio di applicazione java con stub all'esterno della directory xxxx.app
		LOG(1, TEXT(
			"OK posizione esterna soddisfatta.\n"
			"javaAppPath=%s\n"
			"javaAppRoot=%s\n"), javaAppPath.c_str(), javaAppRoot.c_str());

		return Esterno;
	}

	return Indefinito;
}

QString JavaStubApp::findJavaApp(QString __dir)
{
	QStringList dirList = getListDirs(__dir);
	for (QStringList::const_iterator itr = dirList.begin(); itr != dirList.end(); itr++)
	{
		QString f = __dir + L"\\" + (*itr);
		if (itr->length() > 4 && itr->substr(itr->length() - 4) == L".app")
			return f;
	}
	return L"";
}

bool JavaStubApp::readInfoPlist()
{
	FILE *fp;
	if (_wfopen_s(&fp, xmlInfoPath.c_str(), TEXT("rt, ccs=utf-8")) != 0)
	{
		fatalError(TEXT("File info.plist non leggibile."));
		return false;
	}

	// determina dimensione del file
	struct stat st;
	fstat(_fileno(fp), &st);
	int fileLen = st.st_size;

	// legge tutto il file in memoria
	int i = 0, c;
	bufferXmlAllFile = (WCHAR*)malloc((fileLen + 10) * sizeof(WCHAR));
	while ((c = fgetwc(fp)) != EOF && c != 65535)
		bufferXmlAllFile[i++] = c;
	fclose(fp);
	bufferXmlAllFile[i++] = 0;

	// parsing del documento xml	
	doc.parse<0>(bufferXmlAllFile);    // 0 means default parse flags

	XmlNode *root = doc.first_node();
	QString rootName = root->name();
	if (rootName != TEXT("plist"))
	{
		fatalError(TEXT("Il file Info.plist non è conforme."));
		return false;
	}

	XmlNode *dict = root->first_node(TEXT("dict"));
	for (XmlNode *ele = dict->first_node(); ele != NULL; ele = ele->next_sibling())
	{
		QString knome = ele->name();
		if (knome == L"key")
		{
			QString ktext = ele->value();
			LOG(5, L"key %s->%s\n", knome.c_str(), ktext.c_str());

			if (ktext == L"Java")
			{
				if ((ele = ele->next_sibling()) == NULL)
					return false;

				javaProps = ele;
				LOG(5, L"javaProps (%s)\n", ele->name());
			}
			else
			{
				if ((ele = ele->next_sibling()) == NULL)
					break;

				QString snome = ele->name();
				QString stext = ele->value();
				LOG(5, L"string %s->%s\n", snome.c_str(), stext.c_str());

				if (ktext == L"CFBundleName")
				{
					appName = stext;
					setTitle(appName);
					continue;
				}
				if (ktext == L"CFBundleIconFile")
				{
					appIcon = stext;
					continue;
				}
			}
		}
	}

	if (javaProps != NULL)
	{
		WorkingDirectory = javaAppRoot;

		for (XmlNode* ele = javaProps->first_node(); ele != NULL; ele = ele->next_sibling())
		{
			QString knome = ele->name();
			if (knome == L"key")
			{
				QString ktext = ele->value();
				LOG(5, L"key %s->%s\n", knome.c_str(), ktext.c_str());

				if (ktext != L"ClassPath")
				{
					if ((ele = ele->next_sibling()) == NULL)
						break;

					QString snome = ele->name();
					QString stext = ele->value();
					LOG(5, L"string %s->%s\n", snome.c_str(), stext.c_str());

					if (ktext == L"WorkingDirectory")
					{
						WorkingDirectory = replaceAll(stext, L"$APP_PACKAGE", javaAppPath);
						continue;
					}
					if (ktext == L"VMOptions")
					{
						VMOptions = stext;
						continue;
					}
					if (ktext == L"VMOptions-" + stubName)
					{
						VMOptions = stext;
						continue;
					}
					if (ktext == L"MainClass")
					{
						MainClass = stext;
						continue;
					}
					if (ktext == L"MainClass-" + stubName)
					{
						MainClass = stext;
						continue;
					}
					if (ktext == L"JVMVersion")
					{
						JVMVersion = stext;
						continue;
					}
					if (ktext == L"Arguments")
					{
						if (!stext.empty())
							Arguments = replaceAll(replaceAll(stext, L"-mac.", L"-win."), L"$APP_PACKAGE", javaAppPath);
						continue;
					}
					if (ktext == L"ArgumentsWindows")
					{
						if (!stext.empty())
							Arguments = replaceAll(stext, L"$APP_PACKAGE", javaAppPath);
						continue;
					}
					if (ktext == L"Arguments-" + stubName)
					{
						if (!stext.empty())
							Arguments = replaceAll(replaceAll(stext, L"-mac.", L"-win."), L"$APP_PACKAGE", javaAppPath);
						continue;
					}
					if (ktext == L"ArgumentsWindows-" + stubName)
					{
						if (!stext.empty())
							Arguments = replaceAll(stext, L"$APP_PACKAGE", javaAppPath);
						continue;
					}

					if (ktext == L"ClassPathAuto")
					{
						// caricamento automatico della classpath cercando tutti i jars nella directory Resources/Java
						autoScanJars = true;
						autoScanJarsDirectory = replaceAll(stext, L"$JAVAROOT", javaAppRoot + L"/Resources/Java");
					}
				}
				else
				{
					// parsing della classpath
					XmlNode* array = javaProps->first_node(L"array");

					for (XmlNode* ea = array->first_node(); ea != NULL; ea = ea->next_sibling())
					{
						QString stext = replaceAll(ea->value(), L"$JAVAROOT", L"Resources/Java");
						ClassPath.append(L";");
						ClassPath.append(stext);
					}
				}
			}
		}

		// preparazione finale della ClassPath
		if (!ClassPath.empty())
			ClassPath = ClassPath.substr(1);

		if (VMOptions.find(L"-D64") != QString::npos) {
			explicit64bit = true;
			LOG(1, L"VMOptions contiene -D64: solo JVM a 64 bit verranno utilizzate.\n");
		}
		else if (VMOptions.find(L"-D32") != QString::npos) {
			explicit32bit = true;
			LOG(1, L"VMOptions contiene -D32: solo JVM a 32 bit verranno utilizzate.\n");
		}
	}

	return true;
}

bool JavaStubApp::lanciaApplicazione()
{
	if (explicit64bit)
	{
		// richiesta esplicita di una JVM a 64 bit: cerchiamo la più recente a 64 bit
		bool found = false;
		for (JavaInfo ji : jdkList)
		{
			if (ji.bit == 64)
			{
				javaHome = ji.path;
				LOG(1, TEXT("VM64bit: JAVA_HOME=") + javaHome + TEXT("\n"));
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOG(1, TEXT("Nessuna JVM a 64 bit trovata.\n"));
			return false;
		}
	}

	if (explicit32bit)
	{
		// richiesta esplicita di una JVM a 32 bit: cerchiamo la più recente a 32 bit
		bool found = false;
		for (JavaInfo ji : jdkList)
		{
			if (ji.bit == 32)
			{
				javaHome = ji.path;
				LOG(1, TEXT("VM32bit: JAVA_HOME=") + javaHome + TEXT("\n"));
				found = true;
				break;
			}
		}

		if (!found)
		{
			LOG(1, TEXT("Nessuna JVM a 32 bit trovata.\n"));
			return false;
		}
	}

	if (javaPgm.empty())
	{
		QString infoProg = javaHome + L"\\bin\\javaw.exe";
		if (!testForFile(infoProg))
		{
			fatalError(TEXT("La JVM non è stata installata correttamente."));
			return false;
		}
		javaPgm = infoProg;
	}

	ClassPath = replaceAll(ClassPath, L"/", L"\\");
	Arguments = replaceAll(Arguments, L"/", L"\\");
	WorkingDirectory = replaceAll(WorkingDirectory, L"/", L"\\");
	autoScanJarsDirectory = replaceAll(autoScanJarsDirectory, L"/", L"\\");

	int pos;
	if ((pos = javaPgm.find(L"x86")) != QString::npos)
	{
		// siamo a 32 bit: rimuove (riadatta) parametri di lancio per java
		if (VMOptions.empty() || VMOptions.find(L"AUTO_LARGE") != QString::npos)
			VMOptions = L"-D32 -Xms512m -Xmx1200m -XX:+UseParallelGC";
		if (VMOptions.empty() || VMOptions.find(L"AUTO") != QString::npos)
			VMOptions = L"-D32 -Xms512m -Xmx512m -XX:+UseParallelGC";
		LOG(0, L"Selezionata JVM a 32 bit: %s\n", VMOptions.c_str());
	}
	else
	{
		// siamo a 64 bit: rimuove (riadatta) parametri di lancio per java
		if (VMOptions.empty() || VMOptions.find(L"AUTO_LARGE") != QString::npos)
			VMOptions = L"-D64 -Xms512m -Xmx4g -XX:+UseParallelGC";
		if (VMOptions.empty() || VMOptions.find(L"AUTO") != QString::npos)
			VMOptions = L"-D64 -Xms512m -Xmx512m -XX:+UseParallelGC";
		LOG(0, L"Selezionata JVM a 64 bit: %s\n", VMOptions.c_str());
	}

	if(autoScanJars)
		loadClassPathAuto(autoScanJarsDirectory, L"Resources\\java");

	QStringList arguments;
	arguments.append(splitParams(VMOptions));
	arguments.append(L"-cp");
	arguments.append(ClassPath);
	arguments.append(MainClass);
	arguments.append(splitParams(Arguments));

	bool rv = execute(javaPgm, arguments, false, WorkingDirectory);
	if (rv)
		LOG(0, L"Applicazione avviata con successo.\n");

	return rv;
}

bool JavaStubApp::startOperation()
{
	updateCount(10);

	if (testEnvironment() == Indefinito)
	{
		fatalError(TEXT("Uno o più file necessari per l'applicazione non sono stati trovati."));
		return false;
	}

	updateCount(20);

	if (!scanForUpdate())
	{
		fatalError(TEXT("Errore durante l'installazione aggiornamento."));
		return false;
	}

	updateCount(40);

	// legge il file Info.plist
	if (!readInfoPlist())
		return false;

	updateCount(60);

	if (!embeddedJvm())
	{
		// recupera programmi di interesse dal disco
		scanForJavaVM();

		updateCount(70);

		// avvia estrazione della jvm se necessario
		if (javaHome.empty())
		{
			setMessage(L"Loading (JVM) ...");
			extractJvm();
		}
	}

	updateCount(80);

	if (javaHome.empty())
	{
		if(explicit32bit)
			fatalError(TEXT("Non posso utilizzare nessuna JVM a 32 bit valida. Installare una JVM dal sito https://adoptium.net"));
		else if(explicit64bit)
			fatalError(TEXT("Non posso utilizzare nessuna JVM a 64 bit valida. Installare una JVM dal sito https://adoptium.net"));
		else
			fatalError(TEXT("Non posso utilizzare nessuna JVM valida. Installare una JVM dal sito https://adoptium.net"));
		
		return false;
	}

	updateCount(90);

	// avvia l'applicazione
	setMessage(L"Starting application ...");
	if (!lanciaApplicazione())
	{
		fatalError(TEXT("Avvio della JVM non riuscito."));
		return false;
	}

	updateCount(100);
	return true;
}

QStringList JavaStubApp::splitParams(QString param)
{
	return resplit(param);
}

void JavaStubApp::setTitle(QString title)
{
	::setTitle(title.c_str());
}

void JavaStubApp::setMessage(QString message)
{
	::setMessage(message.c_str());
}

void JavaStubApp::updateCount(int val, int total)
{
	int perc = (val * 100) / total;
	::updateCount(perc);
}

void JavaStubApp::updateCount(int val)
{
	::updateCount(val);
}

bool JavaStubApp::execute(QString program, QStringList argument, bool waitExit /*= true*/, QString currentDir /*= L""*/)
{
	program = testQuote(program);

	int i = 1;
	QString cmdLine = program;
	for (QStringList::const_iterator itr = argument.begin(); itr != argument.end(); itr++)
	{
		QString arg = testQuote(*itr);
		cmdLine += L" " + arg;
		LOG(5, L"arg %2d [%s]\n", i++, arg.c_str());
	}

	WCHAR* ptCmdLine = _wcsdup(cmdLine.c_str());
	WCHAR* ptCurrDir = NULL;
	if (!currentDir.empty())
	{
		ptCurrDir = _wcsdup(currentDir.c_str());
		LOG(5, L"Directory corrente in %s\n", currentDir.c_str());
	}

	LOG(5, L"execute: %s\n", ptCmdLine);

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	if (!CreateProcessW(
		NULL,
		ptCmdLine,
		NULL,  //    LPSECURITY_ATTRIBUTES lpProcessAttributes,
		NULL,  //    LPSECURITY_ATTRIBUTES lpThreadAttributes,
		FALSE, //    BOOL                  bInheritHandles,
		NORMAL_PRIORITY_CLASS,     //    DWORD                 dwCreationFlags,
		NULL,  //    LPVOID                lpEnvironment,
		ptCurrDir, //   LPCTSTR               lpCurrentDirectory,
		&si,   //    LPSTARTUPINFO         lpStartupInfo,
		&pi    //    LPPROCESS_INFORMATION lpProcessInformation
		)) {
		int err = GetLastError();
		QString ermsg = describeWindowsError(L"CreateProcessW", err);
		LOG(1, ermsg);
		return false;
	}

	if (waitExit)
		::WaitForSingleObject(pi.hProcess, INFINITE);

	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);
	return true;
}

/**
* Converte una path windows nel formato cygwin.
* La path deve essere assoluta (iniziare per X:\)
* e non deve essere una path UNC (\\server\share\ecc).
* @param path path originale windows
* @return path equivalente cygwin
*/
QString JavaStubApp::convertPathToCygwin(QString path)
{
	std::wregex pw2c(L"^(.):[\\\\/](.+)$");

	std::wcmatch cm;
	if (!std::regex_match(path.c_str(), cm, pw2c))
		return L"";

	if (cm.size() < 2)
		return L"";

	QString sDrive = cm[0];
	QString sPath = cm[1];

	return replaceAll(L"/cygdrive/" + toLowercase(sDrive) + L"/" + sPath, L"\\", L"/");
}

/**
* Convere una path cygwin nel formato windows.
* La path deve iniziare per /cygdrive/x/... e
* @param path
* @return
* @throws Exception
*/
QString JavaStubApp::convertPathFromCygwin(QString path)
{
	std::wregex pc2w(L"^/cygdrive/(.)/(.+)$", std::regex_constants::icase);

	std::wcmatch cm;
	if (!std::regex_match(path.c_str(), cm, pc2w))
		return L"";

	if (cm.size() < 2)
		return L"";

	QString sDrive = cm[0];
	QString sPath = cm[1];

	return replaceAll(sDrive + L":\\" + sPath, L"/", L"\\");
}

bool JavaStubApp::testForFile(QString path)
{
	WIN32_FIND_DATA fdata;
	HANDLE h = FindFirstFile(path.c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	FindClose(h);

	return IS_FILE(fdata.dwFileAttributes);
}

bool JavaStubApp::testForDir(QString path)
{
	WIN32_FIND_DATA fdata;
	HANDLE h = FindFirstFile(path.c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	FindClose(h);

	return IS_DIRECTORY(fdata.dwFileAttributes);
}

QString JavaStubApp::getParent(QString path)
{
	int pos;
	if ((pos = path.find_last_of(L'\\')) != QString::npos)
		return path.substr(0, pos);

	return path;
}

QString JavaStubApp::getFileName(QString path)
{
	int pos;
	if ((pos = path.find_last_of(L'\\')) != QString::npos)
		return path.substr(pos + 1);

	return path;
}

QString JavaStubApp::getFileNameWithoutExt(QString path)
{
	int pos;
	if ((pos = path.find_last_of(L'\\')) != QString::npos)
		path = path.substr(pos + 1);
	if ((pos = path.find_last_of(L'.')) != QString::npos)
		path = path.substr(0, pos);

	return path;
}

QStringList JavaStubApp::getListDirs(QString path)
{
	QStringList rv;
	WIN32_FIND_DATA fdata;
	HANDLE h = FindFirstFile((path + L"\\*.*").c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return rv;

	do
	{
		if (IS_DIRECTORY(fdata.dwFileAttributes))
		{
			QString fileName(fdata.cFileName);

			if (fileName != L"." && fileName != L"..")
			{
				// memorizza per ritorno
				rv.push_back(fileName);
			}
		}

	} while (FindNextFile(h, &fdata));

	FindClose(h);
	return rv;
}

QStringList JavaStubApp::getListFiles(QString path)
{
	QStringList rv;
	WIN32_FIND_DATA fdata;
	HANDLE h = FindFirstFile((path + L"\\*.*").c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return rv;

	do
	{
		if (IS_FILE(fdata.dwFileAttributes))
		{
			// memorizza per ritorno
			rv.push_back(fdata.cFileName);
		}

	} while (FindNextFile(h, &fdata));

	FindClose(h);
	return rv;
}

QString toLowercase(QString data)
{
	int len = data.length() + 16;
	WCHAR *tmp = (WCHAR*)alloca(len * sizeof(WCHAR));
	wcscpy_s(tmp, len, data.c_str());
	_wcslwr_s(tmp, len);
	return tmp;
}

QString toUppercase(QString data)
{
	int len = data.length() + 16;
	WCHAR *tmp = (WCHAR*)alloca(len * sizeof(WCHAR));
	wcscpy_s(tmp, len, data.c_str());
	_wcsupr_s(tmp, len);
	return tmp;
}

QString testQuote(QString in)
{
	if (in.find_first_of(L' ') != QString::npos)
		return L"\"" + in + L"\"";
	return in;
}

QStringList resplit(QString s, QString rgx_str /* = L"\\s+"*/)
{
	QStringList elems;
	std::wregex rgx(rgx_str);
	std::wsregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
	std::wsregex_token_iterator end;

	while (iter != end) {
		//std::cout << "S43:" << *iter << std::endl;
		elems.push_back(*iter);
		++iter;
	}

	return elems;
}

QString replaceAll(const QString& source, const QString& from, const QString& to)
{
	QString newString;
	newString.reserve(source.length());  // avoids a few memory allocations

	QString::size_type lastPos = 0;
	QString::size_type findPos;

	while (QString::npos != (findPos = source.find(from, lastPos)))
	{
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
	}

	// Care for the rest after last occurrence
	newString += source.substr(lastPos);

	return newString;
}

/*
Retrieve the system error message for the last-error code.
*/
QString describeWindowsError(QString function, int errorCode)
{
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	WCHAR buffer[1024];
	swprintf(buffer, sizeof(buffer), L"%s failed with error %d: %s", function.c_str(), errorCode, (wchar_t const*)lpMsgBuf);

	LocalFree(lpMsgBuf);
	return buffer;
}

QString getEnvVar(QString key)
{
	const DWORD buff_size = GetEnvironmentVariableW(key.c_str(), 0, 0);
	WCHAR *buff = (WCHAR*)alloca(buff_size * sizeof(WCHAR));
	const DWORD ret = GetEnvironmentVariableW(key.c_str(), buff, buff_size);
	return buff;
}

bool JavaStubApp::isRunAsAdministrator()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:

	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	if (ERROR_SUCCESS != dwError)
	{
		QString ermsg = describeWindowsError(L"CreateProcessW", dwError);
		LOG(1, ermsg);
		throw dwError;
	}

	return (bool)fIsRunAsAdmin;
}

bool JavaStubApp::elevateNow()
{
	if (isRunAsAdministrator())
		return false;

	QString cmdLine;
	for (int i = 1; i < __argc; i++)
	{
		QString arg = testQuote(__wargv[i]);
		if (i > 1)
			cmdLine += L" ";
		cmdLine += arg;
	}

	SHELLEXECUTEINFO sei = { sizeof(sei) };

	sei.lpVerb = L"runas";
	sei.lpFile = __wargv[0];
	sei.lpParameters = cmdLine.c_str();
	sei.lpDirectory = stubLocation.c_str();
	sei.hwnd = NULL;
	sei.nShow = SW_NORMAL;

	LOG(3, L"Elevate pgm: %s\n", sei.lpFile);
	LOG(3, L"Elevate par: %s\n", sei.lpParameters);
	LOG(3, L"Elevate dir: %s\n", sei.lpDirectory);

	if (!ShellExecuteEx(&sei))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_CANCELLED)
		{
			LOG(1, L"Privilege escalation cancelled by user.");
			return false;
		}

		QString err = describeWindowsError(L"", dwError);
		LOG(1, L"Elevate error: %s\n", err.c_str());
		return false;
	}

	return true;
}

bool JavaStubApp::loadClassPathAuto(QString pathSearch, QString pathStore)
{
	QStringList jars;
	getListFiles(jars, pathSearch, pathStore, L"*.jar", true);

	boolean first = true;
	for (QString var : jars)
	{
		if(!first)
			ClassPath.append(L";");

		ClassPath.append(var);
		first = false;
	}

	return true;
}

bool JavaStubApp::getListFiles(QStringList& topopulate, QString pathSearch, QString pathStore, QString pattern, bool recurse)
{
	WIN32_FIND_DATA fdata;
	HANDLE h = FindFirstFile((pathSearch + L"\\" + pattern).c_str(), &fdata);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	do
	{
		if (IS_FILE(fdata.dwFileAttributes))
		{
			// memorizza per ritorno
			topopulate.push_back(pathStore + L"\\" + fdata.cFileName);
		}

	} while (FindNextFile(h, &fdata));
	FindClose(h);

	if (recurse)
	{
		h = FindFirstFile((pathSearch + L"\\*.*").c_str(), &fdata);
		if (h == INVALID_HANDLE_VALUE)
			return false;

		do
		{
			if (IS_DIRECTORY(fdata.dwFileAttributes))
			{
				QString fileName(fdata.cFileName);

				if (fileName == L"." || fileName == L"..")
					continue;

				// chiamata ricorsiva
				getListFiles(topopulate, pathSearch + L"\\" + fileName, pathStore + L"\\" + fileName, pattern, recurse);
			}

		} while (FindNextFile(h, &fdata));
		FindClose(h);
	}

	return true;
}
