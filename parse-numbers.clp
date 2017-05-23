(deffunction distinct$
             ($?list)
             (bind ?out
                   (create$))
             (progn$ (?a ?list)
                     (if (not (member$ ?a
                                       ?out)) then
                       (bind ?out
                             ?out
                             ?a)))
             ?out)
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
         (declare (salience 10))
         ?f <- (fact ?number&:(numberp ?number))
         =>
         (retract ?f)
         (assert (quodigious-number (value ?number)
                                    (digits (decompose-number ?number)))))

(defrule perform-sum-operation
         ?f <- (quodigious-number (sum FALSE)
                                  (digits $?digits)
                                  (value ?v))
         =>
         (assert (computed-sum ?v))
         (modify ?f (sum (+ (expand$ ?digits)))))

(defrule perform-product-operation
         ?f <- (quodigious-number (product FALSE)
                                  (digits $?digits)
                                  (value ?v))
         =>
         (assert (computed-product ?v))
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



(defrule find-quodigious-numbers-with-same-digits
         ?f <- (quodigious-number (sorted-digits $?a)
                                   (value ?v0))
         (sorted ?v0)
         ?f2 <- (quodigious-number (sorted-digits $?a)
                                    (value ?v1))
         (sorted ?v1)
         (test (neq ?f ?f2))
         =>
         (assert (same-digits ?v0 ?v1)))

(defrule find-quodigious-numbers-with-same-sum-and-product
         ?f <- (quodigious-number (value ?v0)
                                  (sum ?s)
                                  (product ?p))
         ?f2 <- (quodigious-number (value ?v1)
                                   (sum ?s)
                                   (product ?p))
         (test (neq ?f ?f2))
         (computed-product ?v0)
         (computed-sum ?v0)
         (computed-product ?v1)
         (computed-sum ?v1)
         =>
         (assert (same-sum-and-product ?s ?p ?v0 ?v1)))

(defrule find-quodigious-numbers-with-same-sum
         ?f <- (quodigious-number (value ?v0)
                                  (sum ?s))
         ?f2 <- (quodigious-number (value ?v1)
                                   (sum ?s))
         (test (neq ?f ?f2))
         (computed-sum ?v0)
         (computed-sum ?v1)
         =>
         (assert (same-sum ?s ?v0 ?v1)))

(defrule merge-same-sum-and-product
         ?f <- (same-sum-and-product ?s ?p $?rest)
         ?f2 <- (same-sum-and-product ?s ?p $?rest2)
         (test (neq ?f ?f2))
         =>
         (retract ?f ?f2)
         (assert (same-sum-and-product ?s ?p (distinct$ $?rest $?rest2))))

(defrule merge-same-sum
         ?f <- (same-sum ?s $?rest)
         ?f2 <- (same-sum ?s $?rest2)
         (test (neq ?f ?f2))
         =>
         (retract ?f ?f2)
         (assert (same-sum ?s (distinct$ $?rest $?rest2))))


(defrule merge-same-digits
         ?f <- (same-digits $?a ?b $?c)
         ?f2 <- (same-digits $?d ?b $?e)
         (test (neq ?f ?f2))
         =>
         (retract ?f ?f2)
         (assert (same-digits (distinct$ $?a ?b $?c $?d $?e))))
