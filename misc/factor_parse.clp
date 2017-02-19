(deftemplate tagged-factor
             (slot id
                   (default-dynamic (gensym*)))
             (slot value
                   (default ?NONE))
             (slot sum
                   (default ?NONE))
             (slot product
                   (default ?NONE)))
(deftemplate factor-compare
             (slot first-id
                   (type SYMBOL)
                   (default ?NONE))
             (slot second-id 
                   (type SYMBOL)
                   (default ?NONE))
             (slot value-difference
                   (type INTEGER)
                   (default ?NONE))
             (slot sum-difference
                   (type INTEGER)
                   (default ?NONE))
             (slot product-difference
                   (type INTEGER)
                   (default ?NONE)))



(defrule tag-factor 
         ?f <- (factor ?value ?sum ?product)
         =>
         (retract ?f)
         (assert (tagged-factor (value ?value)
                                (sum ?sum)
                                (product ?product))))

(defrule compare-factors
         (tagged-factor (value ?v0)
                        (sum ?s0)
                        (product ?p0)
                        (id ?i0))
         (tagged-factor (value ?v1&~?v0&:(> ?v0 ?v1))
                        (sum ?s1)
                        (product ?p1)
                        (id ?i1))
         =>
         (assert (factor-compare (first-id ?i0)
                                 (second-id ?i1)
                                 (value-difference (- ?v0 ?v1))
                                 (sum-difference (- ?s0 ?s1))
                                 (product-difference (- ?p0 ?p1)))))

(defrule discard-sum-negatives
         (declare (salience 1))
         ?f <- (factor-compare (sum-difference ?sd&:(< ?sd 0)))
         =>
         (retract ?f))
(defrule discard-product-negatives
         (declare (salience 1))
         ?f <- (factor-compare (product-difference ?sd&:(< ?sd 0)))
         =>
         (retract ?f))

(defrule printout-sum-difference-of-one
         (declare (salience -1))
         (factor-compare (sum-difference 1)
                         (first-id ?i0)
                         (second-id ?i1)
                         (value-difference ?sd)
                         (product-difference ?pd))
         =>
         (assert (differences product: ?pd value: ?sd)))

(defrule printout-product-differences-of-zero
         (declare (salience -2))
         (factor-compare (product-difference 0)
                         (first-id ?i0)
                         (second-id ?i1)
                         (value-difference ?vd)
                         (sum-difference ?sd))
         (tagged-factor (id ?i0)
                        (value ?v0))
         (tagged-factor (id ?i1)
                        (value ?v1))
         =>
         (assert ;(value ?v1)
                 (zero-product-difference ?v0 ?v1 sum: ?sd value: ?vd (= (mod ?vd 9) 0))))

(defrule printout-sum-differences-of-zero
         (declare (salience -3))
         (factor-compare (sum-difference 0)
                         (first-id ?i0)
                         (second-id ?i1)
                         (value-difference ?vd)
                         (product-difference ?sd))
         (tagged-factor (id ?i0)
                        (value ?v0))
         (tagged-factor (id ?i1)
                        (value ?v1))
         =>
         (assert ;(value ?v1)
                 (zero-sum-difference ?v0 ?v1 product: ?sd value: ?vd (= (mod ?vd 9) 0))))

(defrule printout-sum-and-product-differences-of-zero
         (declare (salience -4))
         (factor-compare (sum-difference 0)
                         (product-difference 0)
                         (first-id ?i0)
                         (second-id ?i1)
                         (value-difference ?vd))
         (tagged-factor (id ?i0)
                        (value ?v0))
         (tagged-factor (id ?i1)
                        (value ?v1))
         =>
         (assert (zero-sum-and-product-difference ?v0 ?v1 value: ?vd (= (mod ?vd 9) 0))))

(defrule eliminate-indirect-paths-to-zero-sums
        ?f <- (zero-sum-and-product-difference ?end ?start value: $?)
        ?f2 <- (zero-sum-and-product-difference ?mid ?start value: $?)
        ?f3 <- (zero-sum-and-product-difference ?end ?mid value: $?)
        (test (and (neq ?f ?f2 ?f3)
                   (neq ?start ?mid)
                   (neq ?mid ?end)))
        =>
        (retract ?f3))
