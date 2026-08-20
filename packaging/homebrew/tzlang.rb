# ==========================
# Formula de Homebrew para TzLang
# ==========================
#
# Este archivo NO va aqui en su
# destino final: Homebrew lo busca en
# un repositorio aparte llamado
# "tap", que por convencion se llama
#
#   TzLanguaje/homebrew-tzlang
#
# y dentro, en Formula/tzlang.rb
#
# Como publicarlo:
#
#   1. Crea el repo homebrew-tzlang
#      en GitHub.
#
#   2. Copia este archivo a
#      Formula/tzlang.rb dentro de el.
#
#   3. Pon el sha256 del tarball de
#      la version. Se saca asi:
#
#      curl -fsSL https://github.com/TzLanguaje/TzLanguaje/archive/refs/tags/v0.3.2.tar.gz \
#        | shasum -a 256
#
#   4. Los usuarios instalan con:
#
#      brew install tzlanguaje/tzlang/tzlang
#
# Compila desde el codigo en vez de
# descargar el binario: asi la misma
# formula sirve para macOS (Intel y
# Apple Silicon) y para Linux, sin
# mantener cuatro checksums.

class Tzlang < Formula
  desc "Lenguaje de programacion educativo con sintaxis en espanol"
  homepage "https://github.com/TzLanguaje/TzLanguaje"
  url "https://github.com/TzLanguaje/TzLanguaje/archive/refs/tags/v0.3.2.tar.gz"
  sha256 "PON_AQUI_EL_SHA256_DEL_TARBALL"
  license "MIT"
  head "https://github.com/TzLanguaje/TzLanguaje.git", branch: "main"

  depends_on "cmake" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    (testpath/"hola.tz").write <<~TZ
      imprimir "Hola desde TzLang"
    TZ

    assert_equal "Hola desde TzLang",
                 shell_output("#{bin}/tz hola.tz").strip

    assert_match version.to_s, shell_output("#{bin}/tz --version")
  end
end
