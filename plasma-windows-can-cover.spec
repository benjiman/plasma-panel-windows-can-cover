Name:           plasma-windows-can-cover
Version:        0.1.0
Release:        0
Summary:        "Windows can cover" panel mode for Plasma 6 on Wayland
License:        GPL-2.0-or-later
URL:            https://bugs.kde.org/show_bug.cgi?id=484540
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake >= 3.22
BuildRequires:  gcc-c++
BuildRequires:  cmake(LayerShellQt)
BuildRequires:  cmake(Plasma)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6Quick)
# Panel.qml wraps the stock panel from the desktop shell package
Requires:       plasma6-desktop
Requires:       plasma6-workspace
Requires:       layer-shell-qt6

%description
Adds back Plasma 5's "Windows can cover" panel behaviour as an add-on,
without rebuilding any Plasma package. It ships a small QML plugin and an
overlay shell package (falling back to org.kde.plasma.desktop) in which the
"Windows go below" panel visibility mode lets windows cover the panel. The
panel comes back on top when the pointer touches the screen edge.

Run "plasma-windows-can-cover enable" as your user to switch to it.

%prep
%autosetup

%build
%cmake -DQML_INSTALL_DIR=%{_libdir}/qt6/qml
%cmake_build

%install
%cmake_install

%files
%license LICENSE
%doc README.md
%{_bindir}/plasma-windows-can-cover
%dir %{_libdir}/qt6/qml/org
%{_libdir}/qt6/qml/org/windowscancover/
%{_datadir}/plasma/shells/org.windowscancover.desktop/

%changelog
