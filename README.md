# plasma-windows-can-cover

Restores the longtime KDE **"Windows can cover"** panel visibility mode on Plasma 6 / Wayland
(removed upstream, see https://bugs.kde.org/show_bug.cgi?id=484540). It is a separate
package: no stock Plasma package is patched or rebuilt.

Behaviour, as in KWin 5: the panel reserves no space and sits in the normal window stack.
Touch the screen edge under the panel (or hover its visible part) and it comes to the top.
Windows raised or opened after that cover it again; windows that were below it stay below.
While hovered, while one of its popups is open, in edit mode, or while an applet needs
attention, it stays above everything.
<img width="566" height="624" alt="image" src="https://github.com/user-attachments/assets/17253263-e8e4-45a6-bef9-28d726a29767" />
<img width="3785" height="781" alt="image" src="https://github.com/user-attachments/assets/958f51b7-28e2-45ae-be66-bd1aa00f7fa9" />

## Parts

* **QML plugin `org.windowscancover`** (public LayerShellQt + libplasma APIs only):
  * `PanelCover` switches the panel's layer-shell layer: `top` while raised, `bottom` at rest.
  * `EdgeTrigger` is a transparent 1px top-layer strip along the panel's span of the screen
    edge. It only exists while the panel is at rest.
  * `CoverSetting` is the per-panel flag (`windowsCanCover=true` in the panel containment's
    config). The panel's own mode stays "Windows go below", so plasmashell reserves no space.
* **Overlay shell package `org.windowscancover.desktop`** (`X-Plasma-FallbackPackage:
  org.kde.plasma.desktop`) with two files that *wrap* the stock QML instead of copying it:
  * `views/Panel.qml` adds the `PanelCover`.
  * `configuration/PanelConfiguration.qml` adds "Windows can cover" to the Visibility dropdown.
    If a future Plasma changes that dropdown, the stock dialog is shown unchanged.
* **KWin plugin `windowscancover`** turns "layer-shell `bottom` from a plasmashell dock" into
  "normal window layer, stacked just above the window that was topmost when the panel was last
  raised". It does this through a window rule (`windows-can-cover-panel` in `kwinrulesrc`, forcing
  layer Normal for plasmashell docks), which the plugin applies only to panels in this mode.
  KWin never applies rules to layer-shell surfaces on its own, so the rule has no other effect.
  It shows up in System Settings → Window Rules; don't delete it.

## Install / enable

```sh
./build-rpm.sh                      # -> ~/rpmbuild/RPMS/x86_64/plasma-windows-can-cover-*.rpm
sudo zypper in ~/rpmbuild/RPMS/x86_64/plasma-windows-can-cover-*.rpm
plasma-windows-can-cover enable     # per user; restarts plasmashell
# log out and back in (or restart KWin) so KWin loads its plugin
# then: panel > Edit mode > Visibility: "Windows can cover"
```

`enable` sets `[Shell] ShellPackage` in `plasmashellrc`. It also symlinks
`plasma-org.windowscancover.desktop-appletsrc` to your existing
`plasma-org.kde.plasma.desktop-appletsrc`, so both shells share one layout and switching back is
lossless. `plasma-windows-can-cover disable` switches back.

## Upgrades

* **KWin plugin:** KWin only loads plugins built for its exact version, so **rebuild and
  reinstall after every KWin update**. Until you do, KWin skips the plugin and nothing breaks.
  The panel then simply flips between above all windows and below all windows.
* **The rest:** rebuild only when libplasma or layer-shell-qt change soname (rpm's library
  dependencies will flag it).
* Applying a Global Theme with "desktop and window layout" can reset `ShellPackage`. Run
  `enable` again afterwards.
