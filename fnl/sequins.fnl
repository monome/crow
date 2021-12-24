;;; sequins library re-written in fennel

(var s-mt {}) ;to be metatable (redefined to avoid circular dependence)

(fn wrap-index [{: len} ix]
  "fold index into range of a sequins"
  (-> ix
      (- 1)
      (% len)
      (+ 1)))

(fn sequins? [t]
  "return true if the argument is a sequins-table"
  (= s-mt (getmetatable t)))

(fn unwrap-with [f v]
  "if the value is a nested sequins, unwrap it with the supplied fn"
  (if (sequins? v)
      (values (f v)) ;unwrap function can return multiple values
      v))

(fn do-step [{: step &as t} rev]
  "step the index forward destructively, or undo if rev"
  (if (> step.qix 0)
      (do (tset step :qix (- step.qix))
          _)


;; gotta incorporate the tset appropriately
;; try and consolidate the mutation into one spot

;; the general idea is to destructively do-step
;; then if an 'again' occurs, be able to unroll it

;; think (> qix 0) it needs to act
;; (< qix 0) it was just used
;; (= qix 0) it is inactive




    0 _
    )
  (tset step :ix (wrap-index t (if (> step.qix 0)
                                   step.qix
                                   (+ step.ix (unwrap-with realize step.n)))))
  (tset step :qix -1)
  step.ix)

(fn realize [{: data &as self}]
  "realize the next value from a sequins, or signal to its parent"
  ; TODO every
  ; TODO times
  ; TODO count
  ; step & get
  (let [newix (do-step self)])
  (let [newix (wrap-index self (if (> step.qix 0)
                                   step.qix
                                   (+ step.ix (unwrap-with realize step.n))))
        (val flow) (unwrap-with realize (. data newix))]
    (when (not (= flow :again))
      (tset step :ix newix)
      (tset step :qix -1))
    val)) ;TODO turtle

(fn adorn [self mods]
  "add or modify a sequins objects metaparams"
  (if mods ;ignore empty mod tables
      (each [k v (pairs mods)]
        (tset self k :n v))) ;set modifer value (number | fn | sequins)
  self)

(fn new [data mods]
  "create a new sequins object with optional modifiers"
  (-> {:data data
       :len (length data) ;memoize length for speed
       ;; mods
       :step  {:ix 0 :qix 1 :n 1}
       :count {:ix 0 :n 1}
       :every {:ix 0 :n 1}
       :times {:ix 0 :n -1}
       :map   {:n 0}}
      (adorn mods)
      (setmetatable s-mt)))

; initialize the sequins metatable
(set s-mt {:__call realize
           :__len #(. $ :len)})

(setmetatable {: new : adorn}
              {:__call (fn [_ ...] (new ...))})
