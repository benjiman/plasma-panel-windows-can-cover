# plasma-windows-can-cover

Brings back the Plasma 5 **"Windows can cover"** panel mode on Plasma 6 / Wayland
(removed upstream, see https://bugs.kde.org/show_bug.cgi?id=484540). It is a separate
package: no stock Plasma package is patched or rebuilt.

Behaviour: the panel stays visible and reserves no space, but normal windows can sit on top
of it. Touch the screen edge under the panel (or hover the uncovered part) and the panel comes
back on top. It drops back about 150 ms after the pointer leaves, unless one of its popups is
open, the panel settings are open, edit mode is on, or an applet needs attention. The
"activate panel" shortcut raises it for 3 s.

## How it works

* `org.windowscancover` QML plugin (C++, built against the public LayerShellQt and libplasma
  APIs only):
  * `PanelCover` moves the panel's layer-shell surface between the `top` and `bottom` layers.
    KWin restacks layer surfaces when their layer changes, so KWin needs no patch.
  * `EdgeTrigger` is a transparent 1px top-layer strip along the panel's span of the screen
    edge. It only exists while the panel is lowered, so it doesn't take clicks from the panel.
* Overlay shell package `org.windowscancover.desktop` with `X-Plasma-FallbackPackage:
  org.kde.plasma.desktop`. It contains only `views/Panel.qml`, which *wraps* the stock
  `Panel.qml` (`Stock.Panel { PanelCover {...} }`) instead of copying it, so stock updates keep
  flowing through.
* In this shell, the **"Windows go below"** visibility option acts as "Windows can cover"
  (no UI changes needed). The dropdown still shows the old label.

## Install / enable

```sh
./build-rpm.sh                      # -> ~/rpmbuild/RPMS/x86_64/plasma-windows-can-cover-*.rpm
sudo zypper in ~/rpmbuild/RPMS/x86_64/plasma-windows-can-cover-*.rpm
plasma-windows-can-cover enable     # per user; restarts plasmashell
# then: panel > Edit mode > Visibility: "Windows go below"
```

`enable` sets `[Shell] ShellPackage` in `plasmashellrc`. It also symlinks
`plasma-org.windowscancover.desktop-appletsrc` to your existing
`plasma-org.kde.plasma.desktop-appletsrc`, so both shells share one layout and switching back is
lossless. `plasma-windows-can-cover disable` switches back.

Notes:
* Applying a Global Theme with "desktop and window layout" can reset `ShellPackage`. Run
  `enable` again afterwards.
* Rebuild only when libplasma or layer-shell-qt change soname (rpm's automatic
  library dependencies will flag it). Plasma point releases need nothing.
* Stray `Panel.orig.qml` / `Panel.qml.orig` in
  `/usr/share/plasma/shells/org.kde.plasma.desktop/contents/views/` left over from older manual
  hacks aren't owned by any package and can be deleted.
