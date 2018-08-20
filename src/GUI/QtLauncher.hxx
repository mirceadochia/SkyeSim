// QtLauncher.hxx - GUI launcher dialog using Qt5
//
// Written by James Turner, started December 2014.
//
// Copyright (C) 2014 James Turner <zakalawe@mac.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef FG_QTLAUNCHER_HXX
#define FG_QTLAUNCHER_HXX

namespace flightgear
{
  // Only requires FGGlobals to be initialized if 'doInitQSettings' is true.
  // Safe to call several times.
  void initApp(int& argc, char** argv, bool doInitQSettings = true);

  // ensures Qt-related resources are cleaned up. Avoids crashes on shutdown
  // if QPA assets are hanging around. (With the XCB QPA plugin especially)
  void shutdownQtApp();

  // Requires FGGlobals to be initialized. Safe to call several times.
  void initQSettings();

  // Set 'fg-root' in QSettings to the special value "!ask" if either Alt or
  // Shift is pressed. Return true in this case, false otherwise.
  bool checkKeyboardModifiersForSettingFGRoot();

  bool runLauncherDialog();

  bool runInAppLauncherDialog();

  /**
   * @brief restartTheApp quit the application and relaunch it, passing the
   * --launcher flag explicitly.
   */
  void restartTheApp();

  void launcherSetSceneryPaths();
}

#endif // of FG_QTLAUNCHER_HXX
