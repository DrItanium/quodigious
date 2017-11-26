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
(defrule generate-template-bumper
         (declare (salience 10000))
         (unique-numbers ?width
                         values:
                         $?)
         (not (made bumper for ?width))
         =>
         (assert (made bumper for ?width))
         (format t
                 "template<>%ninline void innerBody<%d>(std::ostream& stream, u64 sum, u64 product, u64 index) noexcept {%n"
                 ?width))
(defrule generate-template-bumper-end
         (declare (salience -10000))
         (unique-numbers ?width
                         values:)
         (not (made end bumper for ?width))
         =>
         (assert (made end bumper for ?width))
         (printout t "}" crlf))
        
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
                 "innerBody<0>(stream, sum%s, product%s, index + %d);%n"
                 (make-sum $?digits)
                 (make-product $?digits)
                 ?number))

