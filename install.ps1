<#
==========================
Instalador de TzLang (Windows)
==========================

  irm https://raw.githubusercontent.com/tzerk-last/TzLanguaje/main/install.ps1 | iex

Descarga el binario del ultimo
release, lo verifica contra
SHA256SUMS.txt, lo deja en
%LOCALAPPDATA%\Programs\TzLang\bin
y anade esa carpeta al PATH del
usuario. No hace falta ser
administrador.

Parametros:

  -Version   version concreta (ej. v0.1.0). Por defecto, la ultima.
  -Prefix    carpeta de instalacion.
  -Repo      owner/repo de GitHub.

Ejemplo:

  .\install.ps1 -Version v0.1.0
#>

[CmdletBinding()]
param(
    [string] $Version = $env:TZ_VERSION,
    [string] $Prefix  = $env:TZ_PREFIX,
    [string] $Repo    = $(if ($env:TZ_REPO) { $env:TZ_REPO } else { "tzerk-last/TzLanguaje" })
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

    `$env:TZ_VERSION = "v0.1.0"
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

Info ""
Info "Pruebalo:  tz --version"
