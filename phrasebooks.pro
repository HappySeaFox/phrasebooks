TARGET = phrasebooks
TEMPLATE = app

NVER1=1
NVER2=0
NVER3=0

QT += core gui network widgets

INCLUDEPATH += . qtsingleapplication
DEPENDPATH += . qtsingleapplication

SOURCES += main.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlockedfile.cpp \
    qtsingleapplication/qtlockedfile_win.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    utils.cpp \
    phrasebooks.cpp \
    settings.cpp \
    list.cpp \
    target.cpp \
    miniinput.cpp \
    networkaccess.cpp \
    updatechecker.cpp \
    numericlabel.cpp \
    englishvalidator.cpp \
    about.cpp \
    windowmarker.cpp \
    booksandchapters.cpp \
    selectchapter.cpp \
    chaptersview.cpp \
    options.cpp \
    bookfileiconprovider.cpp

HEADERS += qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtlocalpeer.h \
    utils.h \
    phrasebooks.h \
    settings.h \
    list.h \
    target.h \
    miniinput.h \
    networkaccess.h \
    updatechecker.h \
    numericlabel.h \
    englishvalidator.h \
    about.h \
    windowmarker.h \
    booksandchapters.h \
    selectchapter.h \
    chaptersview.h \
    options.h \
    bookfileiconprovider.h

RESOURCES += phrasebooks.qrc

LIBS += -lpsapi -lgdi32 -luser32

OTHER_FILES += \
    phrasebooks.rc \
    README.txt \
    LICENSE.txt \
    phrasebooks-version.tag

RC_FILE = phrasebooks.rc

include(phrasebooks-common.pri)

# network data
VCSROOT_FOR_DOWNLOAD="http://sourceforge.net/p/phrasebooks/code/ci/master/tree"
HTTPROOT="http://sourceforge.net/p/phrasebooks"
DOWNLOADROOT="http://sourceforge.net/projects/phrasebooks"

# files to copy to the distribution
IMAGEPLUGINS=qico.dll
QTLIBS=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5Network.dll
SSLLIBS=libeay32.dll ssleay32.dll

isEmpty(HOST64) {
    MINGW_BUILD_TYPE=dw2
} else {
    MINGW_BUILD_TYPE=seh
}

MINGWLIBS=libgcc_s_$${MINGW_BUILD_TYPE}-1.dll libwinpthread-1.dll libstdc++-6.dll
LICENSES=LICENSE.txt

DEFINES += VCSROOT_FOR_DOWNLOAD=$$sprintf("\"\\\"%1\\\"\"", $$VCSROOT_FOR_DOWNLOAD)
DEFINES += HTTPROOT=$$sprintf("\"\\\"%1\\\"\"", $$HTTPROOT)
DEFINES += DOWNLOADROOT=$$sprintf("\"\\\"%1\\\"\"", $$DOWNLOADROOT)

tag.commands += $$mle(echo "$$VERSION"> "\"$${_PRO_FILE_PWD_}/phrasebooks-version.tag\"")
tag.commands += $$mle(git commit -m "\"$$VERSION file tag\"" "\"$${_PRO_FILE_PWD_}/phrasebooks-version.tag\"")
tag.commands += $$mle(git tag -a "\"$$VERSION\"" -m "\"$$VERSION\"")
tag.commands += $$mle(git push -u origin master)
QMAKE_EXTRA_TARGETS += tag

!isEmpty(GCCDIR):!isEmpty(ZIP) {
    message("7Z is found, will create custom dist targets")

    # portable binary
    T="$${OUT_PWD}/phrasebooks-portable-$$VERSION"

    distbin.commands += $$mle(if exist \"$$T\" rd /S /Q \"$$T\")
    distbin.commands += $$mle(mkdir \"$$T\")
    distbin.commands += $$mle(mkdir \"$$T/imageformats\")
    distbin.commands += $$mle(mkdir \"$$T/platforms\")
    distbin.commands += $$mle(mkdir \"$$T/translations\")

    # binary & necessary files
    distbin.commands += $$mle(copy /y \"$${OUT_PWD}/$(DESTDIR_TARGET)\" \"$$T\")

    # platforms plugin
    distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qwindows.dll\" \"$$T/platforms\")

    for(ql, QTLIBS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_BINS]\\$$ql\" \"$$T\")
    }

    for(sl, SSLLIBS) {
        distbin.commands += $$mle(copy /y \"$${_PRO_FILE_PWD_}\\extra\\$$sl\" \"$$T\")
    }

    for(ml, MINGWLIBS) {
        distbin.commands += $$mle(copy /y \"$$GCCDIR\\$$ml\" \"$$T\")
    }

    for(ip, IMAGEPLUGINS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\imageformats\\$$ip\" \"$$T/imageformats\")
    }

    distbin.commands += $$mle(copy /y \"$$TRANSLATIONS_DIR\\*.qm\" \"$$T/translations\")

    for(l, LANGUAGES) {
        qtl=$$[QT_INSTALL_TRANSLATIONS]\\qt_$${l}.qm $$[QT_INSTALL_TRANSLATIONS]\\qtbase_$${l}.qm

        for(l, qtl) {
            exists($$l) {
                distbin.commands += $$mle(copy /y \"$$l\" \"$$T/translations\")
            }
        }
    }

    for(lc, LICENSES) {
        distbin.commands += $$mle(copy /y \"$${_PRO_FILE_PWD_}\\$$lc\" \"$$T\")
    }

    # compress
    distbin.commands += $$mle(del /F /Q phrasebooks-portable-$${VERSION}$${HOST64}.zip)
    distbin.commands += $$mle($$ZIP a -r -tzip -mx=9 phrasebooks-portable-$${VERSION}$${HOST64}.zip \"$$T\")
    distbin.commands += $$mle(rd /S /Q \"$$T\")

    QMAKE_EXTRA_TARGETS += distbin
} else {
    warning("7Z is not found, will not create custom dist targets")
}

!isEmpty(GCCDIR):exists($$INNO) {
    message("Inno Setup is found, will create a setup file in a custom dist target")

    INNO_APPID="{{C498F818-43DB-438C-8DAA-272DE94FFCE1}"
    LANGS=$$system(dir /B \"$$INNO\\..\\Languages\")

    ISS="phrasebooks-$${VERSION}.iss"

    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppName \"Phrasebooks\" > $$ISS)
    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppPublisher \"Dmitry Baryshev\" >> $$ISS)
    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppURL \"$$HTTPROOT\" >> $$ISS)

    iss.commands += $$mle(echo [Setup] >> $$ISS)

    !isEmpty(HOST64) {
        iss.commands += $$mle(echo ArchitecturesAllowed=x64 >> $$ISS)
        iss.commands += $$mle(echo ArchitecturesInstallIn64BitMode=x64 >> $$ISS)
    }

    iss.commands += $$mle(echo AppId=$$INNO_APPID >> $$ISS)
    iss.commands += $$mle(echo AppName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo AppVersion=$$VERSION >> $$ISS)
    iss.commands += $$mle(echo AppPublisher={$${LITERAL_HASH}MyAppPublisher} >> $$ISS)
    iss.commands += $$mle(echo AppPublisherURL={$${LITERAL_HASH}MyAppURL} >> $$ISS)
    iss.commands += $$mle(echo AppSupportURL={$${LITERAL_HASH}MyAppURL}/wiki/howto >> $$ISS)
    iss.commands += $$mle(echo AppUpdatesURL={$${LITERAL_HASH}MyAppURL} >> $$ISS)
    iss.commands += $$mle(echo DefaultDirName={pf}\\{$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo DefaultGroupName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo LicenseFile=$${_PRO_FILE_PWD_}\\LICENSE.txt >> $$ISS)
    iss.commands += $$mle(echo OutputDir=. >> $$ISS)
    iss.commands += $$mle(echo OutputBaseFilename=phrasebooks-setup-$${VERSION}$${HOST64} >> $$ISS)
    iss.commands += $$mle(echo SetupIconFile=$${_PRO_FILE_PWD_}\\images\\phrasebooks.ico >> $$ISS)
    iss.commands += $$mle(echo Compression=lzma >> $$ISS)
    iss.commands += $$mle(echo SolidCompression=yes >> $$ISS)
    iss.commands += $$mle(echo UninstallDisplayName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo UninstallDisplayIcon={app}\\$${TARGET}.exe >> $$ISS)
    iss.commands += $$mle(echo MinVersion="0,5.1" >> $$ISS)
    iss.commands += $$mle(echo [Languages] >> $$ISS)

    for(lng, LANGS) {
        lng = $$find(lng, .isl$)

        !isEmpty(lng) {
            lngname=$$lng
            lngname ~= s/\\./x
            iss.commands += $$mle(echo Name: \"$$lngname\"; MessagesFile: \"compiler:Languages\\$$lng\" >> $$ISS)
        }
    }

    iss.commands += $$mle(echo [Tasks] >> $$ISS)
    iss.commands += $$mle(echo Name: \"desktopicon\"; Description: \"{cm:CreateDesktopIcon}\"; GroupDescription: \"{cm:AdditionalIcons}\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"quicklaunchicon\"; Description: \"{cm:CreateQuickLaunchIcon}\"; GroupDescription: \"{cm:AdditionalIcons}\"; OnlyBelowVersion: "0,6.1" >> $$ISS)

    iss.commands += $$mle(echo [Files] >> $$ISS)
    iss.commands += $$mle(echo Source: \"$${OUT_PWD}/$(DESTDIR_TARGET)\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)

    # platforms plugin
    iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\platforms\\qwindows.dll\"; DestDir: \"{app}\\platforms\"; Flags: ignoreversion >> $$ISS)

    for(lc, LICENSES) {
        iss.commands += $$mle(echo Source: \"$${_PRO_FILE_PWD_}\\$$lc\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    iss.commands += $$mle(echo Source: \"$$TRANSLATIONS_DIR/*.qm\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)

    for(l, LANGUAGES) {
        qtl=$$[QT_INSTALL_TRANSLATIONS]\\qt_$${l}.qm $$[QT_INSTALL_TRANSLATIONS]\\qtbase_$${l}.qm

        for(l, qtl) {
            exists($$l) {
                iss.commands += $$mle(echo Source: \"$$l\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)
            }
        }
    }

    for(ql, QTLIBS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_BINS]\\$$ql\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    for(sl, SSLLIBS) {
        iss.commands += $$mle(echo Source: \"$${_PRO_FILE_PWD_}\\extra\\$$sl\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    for(ip, IMAGEPLUGINS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\imageformats\\$$ip\"; DestDir: \"{app}\\imageformats\"; Flags: ignoreversion >> $$ISS)
    }

    for(ml, MINGWLIBS) {
        iss.commands += $$mle(echo Source: \"$$GCCDIR\\$$ml\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    iss.commands += $$mle(echo [Icons] >> $$ISS)
    iss.commands += $$mle(echo Name: \"{group}\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"{group}\\{cm:UninstallProgram","{$${LITERAL_HASH}MyAppName}}\"; Filename: \"{uninstallexe}\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"{commondesktop}\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\"; Tasks: desktopicon >> $$ISS)
    iss.commands += $$mle(echo Name: \"{userappdata}\\Microsoft\\Internet Explorer\\Quick Launch\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\"; Tasks: quicklaunchicon >> $$ISS)

    iss.commands += $$mle(echo [Registry] >> $$ISS)
    iss.commands += $$mle(echo Root: HKLM; Subkey: \"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\"; ValueType: String; ValueName: \"{app}\\$${TARGET}.exe\"; ValueData: \"RUNASADMIN\"; Flags: uninsdeletekeyifempty uninsdeletevalue; MinVersion: "0,6.1" >> $$ISS)

    iss.commands += $$mle(echo [Run] >> $$ISS)
    iss.commands += $$mle(echo Filename: \"{$${LITERAL_HASH}MyAppURL}/wiki/howto\"; Flags: nowait shellexec >> $$ISS)

    iss.commands += $$mle(echo [Code] >> $$ISS)
    iss.commands += $$mle(echo procedure CurStepChanged(CurStep: TSetupStep); >> $$ISS)
    iss.commands += $$mle(echo var >> $$ISS)
    iss.commands += $$mle(echo ResultCode: Integer; >> $$ISS)
    iss.commands += $$mle(echo Uninstall: String; >> $$ISS)
    iss.commands += $$mle(echo UninstallQuery : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo   UninstallQuery := ExpandConstant(\'Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{$${LITERAL_HASH}emit SetupSetting(\"AppId\")}_is1\'); >> $$ISS)

    iss.commands += $$mle(echo   if (CurStep = ssInstall) then begin >> $$ISS)
    iss.commands += $$mle(echo     if RegQueryStringValue(HKLM, UninstallQuery, \'UninstallString\', Uninstall) >> $$ISS)
    iss.commands += $$mle(echo        or RegQueryStringValue(HKCU, UninstallQuery, \'UninstallString\', Uninstall) then begin >> $$ISS)
    iss.commands += $$mle(echo       Uninstall := RemoveQuotes(Uninstall) >> $$ISS)
    iss.commands += $$mle(echo       if (FileExists(Uninstall)) AND (not Exec(RemoveQuotes(Uninstall), \'/VERYSILENT\', \'\', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode)) then begin >> $$ISS)
    iss.commands += $$mle(echo         MsgBox(SysErrorMessage(ResultCode), mbCriticalError, MB_OK); >> $$ISS)
    iss.commands += $$mle(echo         Abort(); >> $$ISS)
    iss.commands += $$mle(echo       end; >> $$ISS)
    iss.commands += $$mle(echo     end; >> $$ISS)
    iss.commands += $$mle(echo   end; >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function FullInstall(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgFullInstallation); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function CustomInstall(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgCustomInstallation); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function ReadyMemoComponents(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgReadyMemoComponents); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    QMAKE_EXTRA_TARGETS += iss
    QMAKE_EXTRA_TARGETS *= distbin

    distbin.depends += iss
    distbin.commands += $$mle(\"$$INNO\" /o. \"$$ISS\")
    distbin.commands += $$mle(del /F /Q \"$$ISS\")
} else {
    warning("Inno Setup is not found, will not create a setup file in a custom dist target")
}

FORMS += \
    phrasebooks.ui \
    list.ui \
    miniinput.ui \
    about.ui \
    booksandchapters.ui \
    selectchapter.ui \
    options.ui
