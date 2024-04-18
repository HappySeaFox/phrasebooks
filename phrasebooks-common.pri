isEmpty(TARGET) {
    error("TARGET is not defined")
}

lessThan(QT_MAJOR_VERSION, 5) {
    error("This project requires Qt 5.3 or greater")
} else {
    equals(QT_MAJOR_VERSION, 5) {
        lessThan(QT_MINOR_VERSION, 3) {
            error("This project requires Qt 5.3 or greater")
        }
    }
}

win32 {
    LANGUAGES=$$system(dir /B \"$${_PRO_FILE_PWD_}\\ts\")
} else {
    LANGUAGES=$$system(ls \"$${_PRO_FILE_PWD_}/ts\")
}

LANGUAGES=$$replace(LANGUAGES, .ts, )

CONFIG += warn_on c++11

VERSION=$$sprintf("%1.%2.%3", $$NVER1, $$NVER2, $$NVER3)

DEFINES += NVER1=$$NVER1
DEFINES += NVER2=$$NVER2
DEFINES += NVER3=$$NVER3
DEFINES += NVER_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

DEFINES += TARGET=$$TARGET

# require at least Windows XP
win32 {
    DEFINES += _WIN32_WINNT=0x0501 WINVER=0x0501
}

# use fast string concat
DEFINES *= QT_USE_QSTRINGBUILDER

contains(QMAKE_HOST.arch, x86_64) {
    HOST64="-x86_64"
}

# search an executable in PATH
defineReplace(findexe) {
    return ( $$system(for %i in ($$1) do @echo. %~$PATH:i) )
}

# escape command to allow multiple lines in Makefile
defineReplace(mle) {
    return ( $$1$$escape_expand(\\n\\t) )
}

# check for gcc
win32-g++ {
    GCC=$$findexe("gcc.exe")
    GCCDIR=$$dirname(GCC)

    isEmpty(GCCDIR) {
        message("MinGW is not found in PATH. Custom dist targets are disabled")
    } else {
        message("MinGW is found in PATH. Custom dist targets are enabled")
    }

    # check for 7z
    ZIP=$$findexe("7z.exe")
}

for(ts, LANGUAGES) {
    win32 {
        MTRANSLATIONS += $${_PRO_FILE_PWD_}\\ts\\$${ts}.ts
    } else {
        MTRANSLATIONS += $${_PRO_FILE_PWD_}/ts/$${ts}.ts
    }
}

message(Translations: $$MTRANSLATIONS)

# copy translations
QMAKE_POST_LINK += $$mle(lupdate -no-obsolete $$_PRO_FILE_ -ts $$MTRANSLATIONS)

# lrelease for each ts
win32 {
    TRANSLATIONS_DIR="$${OUT_PWD}/$(DESTDIR_TARGET)/../translations"
} else {
    TRANSLATIONS_DIR="$${OUT_PWD}/translations"
}

win32 {
    QMAKE_POST_LINK += $$mle(if not exist \"$$TRANSLATIONS_DIR\" mkdir \"$$TRANSLATIONS_DIR\")

    for(ts, LANGUAGES) {
        QMAKE_POST_LINK += $$mle(lrelease \"$${_PRO_FILE_PWD_}\\ts\\$${ts}.ts\" -qm \"$$TRANSLATIONS_DIR/$${ts}.qm\")
    }
} else {
    QMAKE_POST_LINK += $$mle(mkdir -p \"$$TRANSLATIONS_DIR\")

    for(ts, LANGUAGES) {
        QMAKE_POST_LINK += $$mle(lrelease \"$${_PRO_FILE_PWD_}/ts/$${ts}.ts\" -qm \"$$TRANSLATIONS_DIR/$${ts}.qm\")
    }
}

# INNO setup
win32 {
    INNO=$$system(echo %ProgramFiles(x86)%)\\Inno Setup 5\\iscc.exe

    !exists($$INNO) {
        INNO=$$system(echo %ProgramFiles%)\\Inno Setup 5\\iscc.exe
    }
}
