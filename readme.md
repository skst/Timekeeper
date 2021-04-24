# 12noon Timekeeper [![MSBuild](https://github.com/skst/Timekeeper/actions/workflows/msbuild.yml/badge.svg)](https://github.com/skst/Timekeeper/actions/workflows/msbuild.yml)

A desktop tool to display the current date/time on the Windows
task bar in your custom format.

[12noon.com](https://12noon.com)


## .: Features :.

Click on the clock with the primary mouse button to display a calendar.
Click on the clock with the secondary mouse button to display context menu.

Can format clock's display, including:
- Font
- Text color
- Background color
- Alignment: left, center, right
- Data displayed: date format, time format, text, etc.
- Line separators for a multi-line display
- Some pre-formatted date/time entries included, to get the user started.

Timekeeper displays the locale's long date/time in its tooltip.

## .: Calendar :.

Calendar displays current date/time in the caption.
The calendar edges can be dragged to resize the window and display more or fewer months.
You can select a series of dates.
Close the calendar by clicking on the 'x' in the caption or pressing ALT+F4 or ESC.

### Keys
Key            | Action
:------------- | :-----
Home           | First day of current month
End            | Last day of current month
CTRL+Home      | First day of first month on display
CTRL+End       | Last day of last month on display
PageUp         | Move back one month
PageDown       | Move ahead one month
CTRL+PageUp    | Move back one year
CTRL+PageDown  | Move ahead one year
ALT+F4         | Close the calendar
ESC            | Close the calendar

### Calendar options include:

- Show today
- Show week numbers
- Remember calendar's position
- Remember calendar's size


## .: Advanced Features:.

Timekeeper reloads the display format when it notices that it's changed.
You can use this to display your own dynamic value on the desk band.

	HKCU\12noon\Timekeeper\Clock\
			Format [string] = Display format

You can set this data value to any text you'd like to have displayed on the
Timekeeper's desk band. Use it to display the weather, a reminder, whatever.


## .: Building Timekeeper :.

Timekeeper is built with *Visual Studio 2019*.

### Installer

The `build_installer.cmd` script builds the installer using these tools:

- [NSIS3](https://nsis.sourceforge.io)
- [SysInternals SigCheck](https://docs.microsoft.com/en-us/sysinternals/downloads/sigcheck)

The build script assumes they are installed in these folders in the project folder:
- `NSIS3\`
- `SysInternals\`
