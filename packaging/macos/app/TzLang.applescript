-- ==========================
-- TzLang.app
-- ==========================
--
-- Una aplicacion minima cuyo unico
-- trabajo es recibir archivos .tz y
-- ejecutarlos en el Terminal.
--
-- Existe por dos motivos:
--
--   1. En macOS, poner icono a un tipo
--      de archivo SOLO es posible
--      declarandolo desde el
--      Info.plist de una aplicacion.
--      Un programa de linea de ordenes
--      no puede.
--
--   2. De paso da doble clic para
--      ejecutar, igual que en Windows.
--      Para alguien que empieza y no
--      esta comodo con el Terminal,
--      eso vale mas que el icono.

on open listaDeArchivos
	repeat with elemento in listaDeArchivos
		set laRuta to POSIX path of (elemento as text)
		tell application "Terminal"
			activate
			do script "tz " & quoted form of laRuta
		end tell
	end repeat
end open

-- Si alguien abre la aplicacion sola,
-- en vez de soltarle un archivo.
on run
	display dialog "TzLang esta instalado." & return & return & ¬
		"Para ejecutar un programa, haz doble clic sobre un archivo .tz," & return & ¬
		"o escribe en el Terminal:" & return & return & "    tz programa.tz" ¬
		buttons {"Entendido"} default button 1 with title "TzLang"
end run
