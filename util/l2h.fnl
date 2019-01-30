;; Lua to C-header converter

; just stringifys a lua file and wraps it as a char[] in a header

(global hasmeaning (fn [line]
    (if (= nil (string.find line "^%-%-"))   ;; strip lines starting with a comment
        (if (= nil (string.find line "%S"))  ;; strip whitespace lines
            false
            true)
        false)))

(global writeline (fn [src dst]
    (let [nextline (src.read src)]
        (when (~= nextline nil)
            (do (when (= (hasmeaning nextline) true)  ;; check if we can omit the line
                    (dst.write dst (.. "\""
                                       nextline
                                       "\\n\"\n\t")))
                (writeline src dst))))))

(let [filename (. arg 1)]
    (let [f (io.open (.. filename ".h") "w")]
        (f.write f (.. "#pragma once\n\nchar "
                       (string.gsub (string.sub filename
                                                1
                                                -5)
                                    "/"
                                    "_")
                        "[]="))
        (writeline
            (assert (io.open filename "r"))
            f)
        (f.write f ";")
        (f.close f)))
