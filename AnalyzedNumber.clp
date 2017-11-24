(deffunction generate-digits
             (?number ?width)
             (bind ?output
                   (create$))
             (bind ?k
                   ?number)
             (loop-for-count (?ind 1 ?width) do
                             (bind ?output
                                   ?output
                                   (integer (mod ?k 
                                                 10)))
                             (bind ?k
                                   (div ?k
                                        10)))
             ?output)

(deftemplate analyzed-number
             (slot original-value
                   (default ?NONE))
             (multislot digits))

(defrule convert-unique-number
         ?f <- (unique-numbers ?width
                               values:
                               ?number
                               $?rest)
         =>
         (retract ?f)
         (assert (analyzed-number (original-value ?number)
                                  (digits (generate-digits ?number
                                                           ?width)))
                 (unique-numbers ?width
                                 values:
                                 $?rest)))
(deffunction make-sum
             (?digits)
             (bind ?output
                   (create$))
             (progn$ (?a ?digits)
                     (bind ?output
                           ?output
                           + ?a))
             (implode$ ?output))
(deffunction make-product
             (?digits)
             (bind ?output
                   (create$))
             (progn$ (?a ?digits)
                     (bind ?output
                           ?output
                           * ?a))
             (implode$ ?output))
(defrule generate-body-line
         (declare (salience 1))
         ?k <- (analyzed-number (original-value ?number)
                                (digits $?digits))
         =>
         (retract ?k)
         (bind ?k 
               (create$))
         (format t 
                 "innerBody<0>(stream, sum%s, product%s, index + %d, depth);%n"
                 (make-sum $?digits)
                 (make-product $?digits)
                 ?number))

