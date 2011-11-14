[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{b1390e3b1a38d9c94e5041b64b3b8ccb}
;AppId={{1DB6EA4F-D387-432D-A739-283E0E916AF6}
AppName=Screen Message
AppVerName=Screen Message 0.18
AppPublisher=Joachim Breitner
AppPublisherURL=http://www.joachim-breitner.de/projects#screen-message
AppSupportURL=http://www.joachim-breitner.de/projects#screen-message
AppUpdatesURL=http://www.joachim-breitner.de/projects#screen-message
DefaultDirName={pf}\Screen Message
DefaultGroupName=Screen Message
AllowNoIcons=yes
OutputBaseFilename=screen-message-setup
Compression=lzma
SolidCompression=yes
; Is there a point in displaying the LICENSE file?
; LicenseFile=LICENSE
InfoBeforeFile=README.Win32

[Tasks]
Name: quicklaunch; Description: "Add Screen Message icon to the Quick Launch Bar"
Name: hotkey; Description: "Launch Screen Message via Ctrl+Alt+S"

[Files]
Source: "sm.exe"; DestDir: "{app}\bin"
Source: "\etc\*"; DestDir: "{app}\etc"; Flags: recursesubdirs
;Source: "\share\locale\*"; DestDir: "{app}\share\locale"; Flags: recursesubdirs
Source: "\share\themes\*"; DestDir: "{app}\share\themes"; Flags: recursesubdirs
Source: "\bin\libgtk-win32*.dll"; DestDir: "{app}\bin"
Source: "\bin\libgdk-win32*.dll"; DestDir: "{app}\bin"
Source: "\bin\libglib*.dll"; DestDir: "{app}\bin"
Source: "\bin\libgobject*.dll"; DestDir: "{app}\bin"
Source: "\bin\libpango*.dll"; DestDir: "{app}\bin"
Source: "\bin\libgdk_pixbuf*.dll"; DestDir: "{app}\bin"
Source: "\bin\libcairo*.dll"; DestDir: "{app}\bin"
Source: "\bin\libgio*.dll"; DestDir: "{app}\bin"
Source: "\bin\libgmodule*.dll"; DestDir: "{app}\bin"
Source: "\bin\libfontconfig*.dll"; DestDir: "{app}\bin"
Source: "\bin\freetype*.dll"; DestDir: "{app}\bin"
Source: "\bin\libpng*.dll"; DestDir: "{app}\bin"
Source: "\bin\zlib*.dll"; DestDir: "{app}\bin"
Source: "\bin\libexpat*.dll"; DestDir: "{app}\bin"
Source: "\bin\libatk*.dll"; DestDir: "{app}\bin"

[Icons]
Name: "{group}\Screen Message"; Filename: "{app}\bin\sm.exe"; Tasks: not hotkey
Name: "{group}\Screen Message"; Filename: "{app}\bin\sm.exe"; Hotkey: "ctrl+alt+s"; Tasks: hotkey
Name: "{group}\{cm:UninstallProgram,Screen Message}"; Filename: "{uninstallexe}"
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Screen Message"; Filename: "{app}\bin\sm.exe"; Tasks: quicklaunch


[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
