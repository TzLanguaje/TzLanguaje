<#
==========================
Instalador de TzLang (Windows)
==========================

  irm https://raw.githubusercontent.com/TzLanguaje/TzLanguaje/main/install.ps1 | iex

Descarga el binario del ultimo
release, lo verifica contra
SHA256SUMS.txt, lo deja en
%LOCALAPPDATA%\Programs\TzLang\bin
y anade esa carpeta al PATH del
usuario. No hace falta ser
administrador.

Parametros:

  -Version   version concreta (ej. v0.4.1). Por defecto, la ultima.
  -Prefix    carpeta de instalacion.
  -Repo      owner/repo de GitHub.

Ejemplo:

  .\install.ps1 -Version v0.4.1
#>

[CmdletBinding()]
param(
    [string] $Version = $env:TZ_VERSION,
    [string] $Prefix  = $env:TZ_PREFIX,
    [string] $Repo    = $(if ($env:TZ_REPO) { $env:TZ_REPO } else { "TzLanguaje/TzLanguaje" })
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if (-not $Prefix) {
    $Prefix = Join-Path $env:LOCALAPPDATA "Programs\TzLang"
}
$BinDir = Join-Path $Prefix "bin"

function Info  ($m) { Write-Host $m }
function Verde ($m) { Write-Host $m -ForegroundColor Green }
function Morir ($m) { Write-Host "Error: $m" -ForegroundColor Red; exit 1 }

# ==========================
# ARQUITECTURA
# ==========================
#
# Solo se publica x86_64. En ARM64
# funciona igual por emulacion.

$arco = $env:PROCESSOR_ARCHITECTURE
if ($arco -notin @("AMD64", "ARM64", "x86")) {
    Morir "arquitectura no soportada: $arco"
}
$target = "windows-x86_64"

# ==========================
# VERSION
# ==========================

if (-not $Version) {
    Info "Buscando la ultima version..."
    try {
        $api = Invoke-RestMethod "https://api.github.com/repos/$Repo/releases/latest" `
                                 -Headers @{ "User-Agent" = "tzlang-installer" }
        $Version = $api.tag_name
    } catch {
        # Distinguir los tres motivos
        # habituales: sin releases, con
        # el limite de la API agotado, o
        # sin internet. Cada uno se
        # arregla de forma distinta.
        $codigo = $null
        if ($_.Exception.PSObject.Properties['Response'] -and $_.Exception.Response) {
            try { $codigo = [int] $_.Exception.Response.StatusCode } catch { }
        }

        # Ojo: con 'irm ... | iex' NO se
        # pueden pasar parametros, asi
        # que la salida se da con una
        # variable de entorno, que si
        # funciona.
        $comoFijarVersion = @"

Para instalar una version concreta:

    `$env:TZ_VERSION = "v0.4.1"
    irm https://raw.githubusercontent.com/$Repo/main/install.ps1 | iex
"@

        if ($codigo -eq 404) {
            Morir "el repositorio $Repo todavia no tiene ninguna release publicada.$comoFijarVersion"
        } elseif ($codigo -eq 403) {
            Morir "GitHub ha limitado las consultas desde esta red. Espera unos minutos.$comoFijarVersion"
        } else {
            Morir "no se pudo consultar GitHub. Revisa tu conexion a internet.$comoFijarVersion"
        }
    }
}

$archivo = "tzlang-$Version-$target.zip"
$base    = "https://github.com/$Repo/releases/download/$Version"

Info "TzLang $Version  ($target)"

# Ruta al icono de la asociacion. Si
# el paquete no trae el .ico suelto,
# se queda con el que lleva incrustado
# tz.exe: el ',0' es el indice del
# recurso.
$Icono = "$BinDir\tz.exe,0"

# ==========================
# DESCARGAR Y VERIFICAR
# ==========================

$tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("tzlang-" + [Guid]::NewGuid())
New-Item -ItemType Directory -Path $tmp -Force | Out-Null

try {
    $zip = Join-Path $tmp $archivo

    Info "Descargando $archivo..."
    try {
        Invoke-WebRequest "$base/$archivo" -OutFile $zip -UseBasicParsing
    } catch {
        Morir "no se pudo descargar $base/$archivo"
    }

    # El release publica
    # SHA256SUMS.txt. Si esta, se
    # comprueba.
    $sums = Join-Path $tmp "SHA256SUMS.txt"
    try {
        Invoke-WebRequest "$base/SHA256SUMS.txt" -OutFile $sums -UseBasicParsing
    } catch {
        $sums = $null
    }

    if ($sums -and (Test-Path $sums)) {
        $suma  = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
        $linea = Get-Content $sums | Where-Object { $_ -match [regex]::Escape($archivo) } | Select-Object -First 1

        if (-not $linea) {
            Info "Aviso: $archivo no aparece en SHA256SUMS.txt, no se verifica."
        } else {
            $esperada = ($linea -split '\s+')[0].ToLower()
            if ($suma -ne $esperada) {
                Morir "el checksum no coincide. Descarga corrupta o manipulada; no se instala nada."
            }
            Info "Checksum verificado."
        }
    }

    # ==========================
    # INSTALAR
    # ==========================

    $extraido = Join-Path $tmp "out"
    Expand-Archive -Path $zip -DestinationPath $extraido -Force

    $origen = Get-ChildItem -Path $extraido -Filter "tz.exe" -Recurse |
              Select-Object -First 1
    if (-not $origen) { Morir "el paquete no contiene tz.exe" }

    New-Item -ItemType Directory -Path $BinDir -Force | Out-Null

    # Si hay un tz.exe corriendo, la
    # copia falla con un mensaje que
    # no dice nada. Mejor explicarlo.
    try {
        Copy-Item $origen.FullName (Join-Path $BinDir "tz.exe") -Force
    } catch {
        Morir "no se pudo escribir en $BinDir. Cierra cualquier ventana que este usando tz.exe y reintenta."
    }

    # El .zip trae tambien el icono.
    # Los releases viejos no lo llevan:
    # entonces se usa el que va dentro
    # del propio tz.exe.
    $ico = Get-ChildItem -Path $extraido -Filter "tzlang.ico" -Recurse |
           Select-Object -First 1
    if ($ico) {
        Copy-Item $ico.FullName (Join-Path $Prefix "tzlang.ico") -Force
        $Icono = Join-Path $Prefix "tzlang.ico"
    }

    Verde "TzLang instalado en $BinDir\tz.exe"

} finally {
    Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

# ==========================
# PATH
# ==========================
#
# PATH del USUARIO, no del sistema:
# no hace falta elevar permisos.

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if (-not $userPath) { $userPath = "" }

$yaEsta = $userPath -split ';' |
          Where-Object { $_.TrimEnd('\') -ieq $BinDir.TrimEnd('\') }

if (-not $yaEsta) {
    $nuevo = if ($userPath.TrimEnd(';')) { "$($userPath.TrimEnd(';'));$BinDir" } else { $BinDir }
    [Environment]::SetEnvironmentVariable("Path", $nuevo, "User")
    $env:Path = "$env:Path;$BinDir"
    Info ""
    Info "$BinDir anadido a tu PATH."
    Info "Abre una terminal NUEVA para que surta efecto."
}

# ==========================
# ICONO Y DOBLE CLIC
# ==========================
#
# Esto es lo que hace el instalador
# .exe y aqui faltaba: sin estas claves
# los .tz salen con el icono en blanco
# y el doble clic no sabe que hacer con
# ellos.
#
# Todo va en HKCU: es el usuario, no la
# maquina, asi que no hace falta
# elevar permisos.

$Clases = "HKCU:\Software\Classes"
$ProgId = "TzLang.Programa"

function Clave ($ruta, $valor) {
    if (-not (Test-Path $ruta)) {
        New-Item -Path $ruta -Force | Out-Null
    }
    # '(default)' es el nombre que da el
    # proveedor de registro al valor
    # predeterminado, el que sale sin
    # nombre en regedit.
    Set-ItemProperty -Path $ruta -Name "(default)" -Value $valor
}

try {
    Clave "$Clases\.tz"                        $ProgId
    Clave "$Clases\$ProgId"                    "Programa de TzLang"
    Clave "$Clases\$ProgId\DefaultIcon"        $Icono
    Clave "$Clases\$ProgId\shell\open\command" ('"{0}\tz.exe" "%1"' -f $BinDir)

    # ==========================
    # AVISAR AL EXPLORADOR
    # ==========================
    #
    # El Explorador lee las asociaciones
    # al arrancar y no vuelve a mirar.
    # Sin este aviso las claves quedan
    # bien escritas y los iconos siguen
    # en blanco hasta cerrar sesion.
    #
    # SHCNE_ASSOCCHANGED = 0x08000000
    # SHCNF_IDLIST       = 0x0000

    try {
        if (-not ("TzLang.Shell" -as [type])) {
            Add-Type -Namespace TzLang -Name Shell -MemberDefinition @"
[System.Runtime.InteropServices.DllImport("shell32.dll")]
public static extern void SHChangeNotify(
    int eventId, uint flags, System.IntPtr item1, System.IntPtr item2);
"@
        }
        [TzLang.Shell]::SHChangeNotify(0x08000000, 0x0000, [IntPtr]::Zero, [IntPtr]::Zero)
        Verde "Los archivos .tz ya tienen el icono de TzLang."
    } catch {
        Info "Los .tz quedan asociados, pero el Explorador no se ha enterado todavia."
        Info "Cierra la sesion y vuelve a entrar para ver el icono."
    }

    # ==========================
    # ELECCION PREVIA DEL USUARIO
    # ==========================
    #
    # Si alguna vez se abrio un .tz con
    # 'Abrir con' y se marco la casilla,
    # Windows guarda esa eleccion aqui y
    # MANDA sobre lo que acabamos de
    # escribir: el icono seguira siendo
    # el del Bloc de notas. No se puede
    # quitar desde un script (la clave va
    # firmada a proposito), asi que solo
    # se avisa.

    $eleccion = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.tz\UserChoice"

    if (Test-Path $eleccion) {
        Info ""
        Info "Aviso: los .tz estan abiertos con otro programa por eleccion tuya,"
        Info "y esa eleccion manda sobre el icono. Para deshacerla: clic derecho"
        Info "en un .tz > Abrir con > Elegir otra aplicacion > TzLang > Siempre."
    }

} catch {
    Info "No se pudo asociar los archivos .tz: $($_.Exception.Message)"
    Info "TzLang funciona igual desde la terminal."
}

Info ""
Info "Pruebalo:  tz --version"
