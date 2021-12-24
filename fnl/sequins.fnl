;;; sequins library re-written in fennel

(var s-mt {}) ;to be metatable (redefined to avoid circular dependence)

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

;every = function(f,n) return (f.ix % n) ~= 0 end,
;function do_flow(s, k)
;    local f = s.flw[k] -- check if times exists
;    if f then
;        f.ix = f.ix + 1
;        return S.flows[k](f, turtle(f.n))
;    end
;end
;if do_flow(s, 'every') then return 'skip' end

(fn do-every-with [f self]
  )

(fn do-step-with [f {: step &as self}]
  "advance index by step amount"
  ;TODO if we use mod-base1 instead just destructure :len directly
  (let [ix (wrap-index self (if (> step.qix 0)
                                step.qix
                                (+ step.ix (unwrap-with f step.n))))]
    (tset step :ix ix)
    (tset step :qix -1)
    ix))

(fn realize [{: data &as self}]
  "realize the next value from a sequins, or signal to its parent"
  ; TODO every
  ; TODO times
  ; TODO count
  ; step & get
  (let [newix (do-step-with realize self)
        (val flow) (unwrap-with realize (. data newix))]
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
