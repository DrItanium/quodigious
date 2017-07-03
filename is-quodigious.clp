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
               (bind ?curr 
                (integer (mod ?value
                          10)))
               (bind ?next
                     (div ?value
                          10))
               (create$ ?curr
                        (decompose-number ?next))))

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
         (declare (salience 1))
         ?f <- (quodigious-number (sum FALSE)
                                  (digits $?digits)
                                  (value ?v))
         =>
         (assert (computed-sum ?v))
         (modify ?f (sum (+ (expand$ ?digits)))))

(defrule perform-product-operation
         (declare (salience 1))
         ?f <- (quodigious-number (product FALSE)
                                  (digits $?digits)
                                  (value ?v))
         =>
         (assert (computed-product ?v))
         (modify ?f (product (* (expand$ ?digits)))))

(defrule is-quodigious
         (computed-sum ?v)
         (computed-product ?v)
         (quodigious-number (value ?v)
                            (product ?p&:(numberp ?p))
                            (sum ?s&:(numberp ?s)))
         (test (or (<> (mod ?v ?p) 0)
                   (<> (mod ?v ?s) 0)))
         =>
         (printout t ?v " is not quodigious!" crlf))

