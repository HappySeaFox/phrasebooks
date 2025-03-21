#
# spec file for package phrasebooks
#
# Copyright (c) 2016 Dmitry Baryshev <dima8w@gmail.com>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#
Name: phrasebooks
Summary: English phrasebooks manager
License: GPL-3.0
Group: Applications/Education
URL: https://github.com/HappySeaFox/phrasebooks
Version: 1.0.1
Release: 12
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-build

%if 0%{?suse_version}
BuildRequires: libqt5-qtbase-devel libqt5-qttools-devel libQt5X11Extras-devel libXext-devel libXtst-devel libX11-devel libXt-devel
%endif

%if 0%{?fedora_version}
BuildRequires: qt5-qtbase-devel qt5-qtx11extras-devel qt5-linguist libXext-devel libXtst-devel libX11-devel libXt-devel
%endif

%description
The purpose of this applications is to manage English phrasebooks
and load the specified English word or text into an external dictionary.

The application DOES NOT translate a text, it only loads it into an external
dictionary to get the full translation with synonyms, examples, articles etc.

The application is intended to a person who'd like to learn an English
vocabulary and widen his/her English lexicon (other languages are not supported
at this time), but his/her favorite dictionary doesn't support advanced
phrasebooks.

If your favorite dictionary supports advanced phrasebooks, then
you don't need this application.

%prep
%setup
%build
qmake-qt5 %{name}.pro
make
%install
install -D -m 755 phrasebooks $RPM_BUILD_ROOT/usr/bin/phrasebooks
install -d -m 755 $RPM_BUILD_ROOT/usr/share/phrasebooks/translations
find ts -name '*.ts' -type f -exec lrelease-qt5 '{}' ';'
find ts -name '*.qm' -type f -exec install -m 644 '{}' $RPM_BUILD_ROOT/usr/share/phrasebooks/translations/ ';'
install -D -m 644 icons/phrasebooks-16.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/16x16/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-24.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/24x24/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-32.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/32x32/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-48.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/48x48/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-64.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/64x64/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-72.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/72x72/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-96.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/96x96/apps/phrasebooks.png
install -D -m 644 icons/phrasebooks-128.png $RPM_BUILD_ROOT/usr/share/icons/hicolor/128x128/apps/phrasebooks.png
install -D -m 644 man/phrasebooks.1 $RPM_BUILD_ROOT/usr/share/man/man1/phrasebooks.1
install -D -m 644 phrasebooks.desktop $RPM_BUILD_ROOT/usr/share/applications/phrasebooks.desktop
%files
%defattr(-,root,root,-)
/usr/bin/phrasebooks
/usr/share/icons/hicolor/16x16
/usr/share/icons/hicolor/16x16/apps
/usr/share/icons/hicolor/16x16/apps/phrasebooks.png
/usr/share/icons/hicolor/24x24
/usr/share/icons/hicolor/24x24/apps
/usr/share/icons/hicolor/24x24/apps/phrasebooks.png
/usr/share/icons/hicolor/32x32
/usr/share/icons/hicolor/32x32/apps
/usr/share/icons/hicolor/32x32/apps/phrasebooks.png
/usr/share/icons/hicolor/48x48
/usr/share/icons/hicolor/48x48/apps
/usr/share/icons/hicolor/48x48/apps/phrasebooks.png
/usr/share/icons/hicolor/64x64
/usr/share/icons/hicolor/64x64/apps
/usr/share/icons/hicolor/64x64/apps/phrasebooks.png
/usr/share/icons/hicolor/72x72
/usr/share/icons/hicolor/72x72/apps
/usr/share/icons/hicolor/72x72/apps/phrasebooks.png
/usr/share/icons/hicolor/96x96
/usr/share/icons/hicolor/96x96/apps
/usr/share/icons/hicolor/96x96/apps/phrasebooks.png
/usr/share/icons/hicolor/128x128
/usr/share/icons/hicolor/128x128/apps
/usr/share/icons/hicolor/128x128/apps/phrasebooks.png
/usr/share/phrasebooks
/usr/share/phrasebooks/translations
/usr/share/phrasebooks/translations/*.qm
/usr/share/man/man1/phrasebooks.1.gz
/usr/share/applications/phrasebooks.desktop
%changelog
* Mon Apr 25 2016 Dmitry Baryshev <dima8w@gmail.com> 1.0.1-1
- Added "Always on top" setting
- Removed the green "OK" label
* Tue Apr 19 2016 Dmitry Baryshev <dima8w@gmail.com> 1.0.0-11
- General SPEC fixes
* Tue Apr 19 2016 Dmitry Baryshev <dima8w@gmail.com> 1.0.0-10
- Initial RPM release