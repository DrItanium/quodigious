(deffunction make-fact
             ($?contents)
             (assert (fact $?contents)))

(deffunction assert-facts-from-file
             (?file)
             (if (open ?file foo "r") then
               (while (neq (bind ?line
                                 (readline foo))
                           EOF) do
                      (make-fact (explode$ ?line)))
               (close foo)
               TRUE
               else
               FALSE))

(defrule identify-sum
         (fact value: ?value $? sum: ?sum $?)
         =>
         (assert (sum ?sum ?value)))
(defrule identify-product
         (fact value: ?value $? product: ?product $?)
         =>
         (assert (product ?product ?value)))
(defrule ignore-straight-numbers
         ?f <- (fact ?number&:(numberp ?number))
         =>
         (retract ?f))

(defrule ignore-empty-facts
         ?f <- (fact)
         =>
         (retract ?f))

(defrule ignore-sum-is-odd
         ?f <- (fact Sum is odd!)
         =>
         (retract ?f))

(defrule ignore-product-is-odd
         ?f <- (fact Product is odd!)
         =>
         (retract ?f))

(defrule merge-products
         ?f <- (product ?product $?a)
         ?f2 <- (product ?product $?b)
         (test (neq ?f
                    ?f2))
         =>
         (retract ?f
                  ?f2)
         (assert (product ?product $?a $?b)))

(defrule merge-sums
         ?f <- (sum ?sum $?a)
         ?f2 <- (sum ?sum $?b)
         (test (neq ?f
                    ?f2))
         =>
         (retract ?f
                  ?f2)
         (assert (sum ?sum $?a $?b)))
