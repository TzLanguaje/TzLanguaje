; ==========================
; Instalador de TzLang para Windows
; ==========================
;
; Genera un .exe con asistente, igual
; que el de python.org: pantallas de
; Siguiente, casilla para anadir al
; PATH, asociacion de los archivos
; .tz y entrada en "Agregar o quitar
; programas".
;
; Se compila con Inno Setup 6:
;
;   iscc /DTzVersion=0.1.0 /DTzBinario=..\..\build-rel\Release\tz.exe packaging\windows\tzlang.iss
;
; Inno Setup es gratuito. En los
; runners de GitHub se instala con
;   choco install innosetup
;
; Parametros que espera:
;
;   TzVersion   la version, sin la 'v'
;   TzBinario   ruta al tz.exe ya compilado
;   TzSalida    carpeta donde dejar el instalador (por defecto: dist)

#ifndef TzVersion
  #error Falta /DTzVersion=X.Y.Z
#endif

#ifndef TzBinario
  #error Falta /DTzBinario=ruta\a\tz.exe
#endif

#ifndef TzSalida
  #define TzSalida "dist"
#endif

#define TzNombre "TzLang"
#define TzUrl    "https://github.com/tzerk-last/TzLanguaje"

[Setup]
AppId={{8F3A6C21-4B7D-4E9A-9C15-2D8E7A0B3F44}
AppName={#TzNombre}
AppVersion={#TzVersion}
AppVerName={#TzNombre} {#TzVersion}
AppPublisher=tzerk-last
AppPublisherURL={#TzUrl}
AppSupportURL={#TzUrl}/issues
AppUpdatesURL={#TzUrl}/releases

; Instala en la carpeta del usuario,
; no en Archivos de programa: asi NO
; hace falta ser administrador, que
; es lo que hace Python por defecto.
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

DefaultDirName={autopf}\{#TzNombre}
DefaultGroupName={#TzNombre}
DisableProgramGroupPage=yes
DisableDirPage=no

LicenseFile=..\..\LICENSE
InfoAfterFile=despedida.txt

OutputDir={#TzSalida}
OutputBaseFilename=TzLang-v{#TzVersion}-windows-x86_64-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

; El binario es de 64 bits.
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

UninstallDisplayName={#TzNombre} {#TzVersion}
UninstallDisplayIcon={app}\bin\tz.exe

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
; Marcada por defecto, como la de
; Python. Es lo que hace que 'tz'
; funcione desde cualquier terminal.
Name: "modifypath"; \
    Description: "Anadir TzLang al PATH (recomendado)"; \
    GroupDescription: "Integracion con el sistema:"

; Asocia los .tz para poder
; ejecutarlos con doble clic.
Name: "associate"; \
    Description: "Asociar los archivos .tz con TzLang"; \
    GroupDescription: "Integracion con el sistema:"

[Files]
Source: "{#TzBinario}";   DestDir: "{app}\bin"; DestName: "tz.exe"; Flags: ignoreversion
Source: "..\..\LICENSE";   DestDir: "{app}";     DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\..\README.md"; DestDir: "{app}";     Flags: ignoreversion

[Registry]
; ==========================
; PATH
; ==========================
;
; Inno anade la carpeta al PATH del
; usuario y quita la entrada al
; desinstalar. No pisa el PATH
; existente: lo extiende.
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}\bin"; \
    Check: NecesitaPath(ExpandConstant('{app}\bin')); \
    Tasks: modifypath

; ==========================
; ASOCIACION DE .tz
; ==========================
Root: HKCU; Subkey: "Software\Classes\.tz"; \
    ValueType: string; ValueName: ""; ValueData: "TzLang.Programa"; \
    Flags: uninsdeletevalue; Tasks: associate

Root: HKCU; Subkey: "Software\Classes\TzLang.Programa"; \
    ValueType: string; ValueName: ""; ValueData: "Programa de TzLang"; \
    Flags: uninsdeletekey; Tasks: associate

Root: HKCU; Subkey: "Software\Classes\TzLang.Programa\DefaultIcon"; \
    ValueType: string; ValueName: ""; ValueData: "{app}\bin\tz.exe,0"; \
    Tasks: associate

Root: HKCU; Subkey: "Software\Classes\TzLang.Programa\shell\open\command"; \
    ValueType: string; ValueName: ""; ValueData: """{app}\bin\tz.exe"" ""%1"""; \
    Tasks: associate

[Icons]
Name: "{group}\Documentacion de TzLang"; Filename: "{app}\README.md"
Name: "{group}\Sitio web de TzLang";     Filename: "{#TzUrl}"

[Code]
{ ==========================
  ¿Hace falta anadir la carpeta?
  ==========================

  Si el usuario reinstala, el PATH ya
  la contiene y volver a anadirla la
  duplicaria. }

function NecesitaPath(Carpeta: string): Boolean;
var
  PathActual: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', PathActual) then
  begin
    Result := True;
    exit;
  end;

  { Se compara en minusculas y con ';'
    alrededor para no confundir
    C:\TzLang\bin con C:\TzLang\bin2 }
  Result :=
    Pos(';' + Lowercase(Carpeta) + ';', ';' + Lowercase(PathActual) + ';') = 0;
end;
