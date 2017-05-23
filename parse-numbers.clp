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

(deffunction decompose-number
             (?value)
             (if (< ?value 0) then
               (return FALSE))
             (if (= ?value 0) then
               (create$)
               else
               (create$ (integer (mod ?value
                                      10))
                        (decompose-number (integer (/ ?value
                                                      10))))))

(deftemplate quodigious-number
             (slot value
                   (type INTEGER)
                   (default ?NONE))
             (slot sum
                   (type INTEGER
                         SYMBOL)
                   (default FALSE))
             (slot product
                   (type INTEGER
                         SYMBOL)
                   (default FALSE))
             (multislot sorted-digits
                        (type INTEGER))
             (multislot digits
                        (type INTEGER)))


(defrule build-quodigious-number
         ?f <- (fact ?number&:(numberp ?number))
         =>
         (retract ?f)
         (assert (quodigious-number (value ?number)
                                    (digits (decompose-number ?number)))))

(defrule perform-sum-operation
         ?f <- (quodigious-number (sum FALSE)
                                  (digits $?digits))
         =>
         (modify ?f (sum (+ (expand$ ?digits)))))

(defrule perform-product-operation
         ?f <- (quodigious-number (product FALSE)
                                  (digits $?digits))
         =>
         (modify ?f (product (* (expand$ ?digits)))))

(defrule perform-sort
         ?f <- (quodigious-number (sorted-digits)
                                  (value ?v)
                                  (digits $?digits))
         (not (sorted ?v))
         =>
         (assert (sorted ?v))
         (modify ?f
                 (sorted-digits (sort <
                                      $?digits))))



