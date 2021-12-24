;;; sequins library re-written in fennel

(var s-mt {}) ;to be metatable (redefined to avoid circular dependence)

(fn mod-1 [ix wrap]
  "modulo for 1-based vals"
  (-> ix
      (- 1)
      (% wrap)
      (+ 1)))

;; FIXME should this just be a modulo-base1 fn & remove the need for a self?
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

;; simplify this by breaking .qix into it's own modifier :select (overrides :step)
;; then do-step can also use (inc-flow) as part of (flow-with)
(fn do-step-with [f {: step : len : data}]
  "advance index by step amount"
  (let [ix (mod-1 (if (> step.qix 0)
                      step.qix ;index override
                      (+ step.ix (unwrap-with f step.n)))
                  len)]
    (tset step :ix ix)
    (tset step :qix -1)
    (values (unwrap-with f (. data ix)))))

(local flows {:every #(= 0 (% $1 $2))
              :times #(<= $1 $2)
              :count #true})

(fn flow-with [key f self]
  (let [flow (. self key)]
    (tset flow :ix (+ 1 (. flow :ix)))
    (let [{: ix : n} flow]
      ((. flows key) ix n))))

(fn r [self]
  "realize the next value from a sequins, or signal to its parent"
  (match (flow-with :every r self)
    false (values nil :skip)
    _ (match (flow-with :times r self)
        false (values nil :dead)
        ;_ (let [again (do-count-with r self)
        _ (let [again (flow-with :count r self)
                (v flow) (do-step-with r self)]
            ;; TODO unroll count action (:again)
            (if (or (= flow :skip) (= flow :dead))
                (values (unwrap-with r self))
                (values v again))))))

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
       :times {:ix 0 :n 99999999} ;; a really big number
       :map   {:n 0}}
      (adorn mods)
      (setmetatable s-mt)))

; initialize the sequins metatable
(set s-mt {:__call r
           :__len #(. $ :len)})

(setmetatable {: new : adorn}
              {:__call (fn [_ ...] (new ...))})
