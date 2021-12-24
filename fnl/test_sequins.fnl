;;; sequins.fnl tester

(local fennel (require :fennel))
(local s* (require :sequins))

(fn stest [str s expectations]
  "take a sequins & test sequential accesses aganst a list"
  (each [k v (ipairs expectations)]
    (match (s)
      v nil ;passed test
      _ (do (print (.. "error in: " str " @[" k "]. expected " v " but got " (tostring _)))
            ;(print (fennel.view s)))))) ;uncomment for debug at each failed call
            ))))

(stest "basic sequential list" ;name of test
       (s* [0 4 7 11])         ;sequins under test
       [0 4 7 11 0])           ;expected series of results

(stest "step in reverse"
       (s* [0 3 6 9]
           {:step -1})
       [0 9 6 3 0])

(stest "nested sequins"
       (s* [1 (s* [2 3])])
       [1 2 1 3 1 2])

(stest "sequins as step"
       (s* [1 2 3]
           {:step (s* [1 1 -1])})
       [1 2 3 2 3 1 3])

(stest "nested sequins with every"
       (s* [1 (s* [2]
              {:every 2})])
       [1 1 2 1 1 2 1])

(stest "nested sequins with times"
       (s* [1 (s* [2]
              {:times 2})])
       [1 2 1 2 1 1 1])
;;(local s19 (s* [1 (s* [2 3]
;;                      {:times 3})]))

;;(local s9 (s* [1 (s* [2]
;;                     {:count 2})]))
;;
;;(local s11 (s* [1 (s* [2]
;;                      {:every 2
;;                       :count 3})]))
;;
;;;;; naming nested sequins
;;(let [melody (s* [3 4 5] {:count 3})
;;      umelo (s* [-3 -4 -5] {:count 3})
;;      s15 (s* [melody (s*! umelo
;;                           {:every 3})])])
;;



;;;;; update & query single elements like a regular table
;;;;; TODO query if this makes sense in fnl?
;;(local s13 (s* [1 2 3]))
;;(assert (= (. s13 2) 2))
;;(tset s13 1 4)
;;
;;;;; update the whole table
;;(local s14 (s* [1 2 3 4]))
;;(s14:settable [5 6])
;;
;;
;;;; reassign next behaviour
;;(local s5 (s* [1 2 3 4]))
;;(s*! s5 {:step -1})
;;; or OO style
;;(s5:step -1)
;;
;;;; test reset
;;(local s18 (s* [1 (s* [2 3]
;;                      {:count 2})]))
;;(s18:reset)
;;
;;;; string tests
;;(local s1 "cba")
;;
;;;; __len test
;;(local s1 (s* [1 2 3]))
;;(assert (length s1) 3)
;;
