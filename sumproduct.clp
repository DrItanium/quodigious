(deftemplate qnum-stats
             (slot length
                   (type INTEGER)
                   (default ?NONE))
             (slot line
                   (type LEXEME)
                   (default ?NONE))
             (slot sum
                   (type INTEGER)
                   (default ?NONE))
             (slot product
                   (type INTEGER)
                   (default ?NONE)))
(deffunction get-digits
             (?str)
             (bind ?digits
                   (create$))
             (loop-for-count (?a 1 
                                 (str-length ?str)) do
                             (bind ?digits
                                   ?digits
                                   (string-to-field (sub-string ?a ?a
                                                                ?str))))
             ?digits)

(deffunction digit-sum
             (?str)
             (+ 0 (expand$ (get-digits ?str))))
(deffunction digit-product
             (?str)
             (* 1 (expand$ (get-digits ?str))))

(deffunction digit-sum-product
             (?str)
             (create$ ;(string-to-field ?str)
               (digit-sum ?str)
               (digit-product ?str)))

(deffunction sum-digit-from-file-to-facts
             (?path)
             (if (open ?path 
                       (bind ?name 
                             (gensym*))
                       "r") then
               (while (neq (bind ?line
                                 (readline ?name))
                           EOF) do
                      (if (> (str-length ?line) 
                             0) then
                        (bind ?collec
                              (digit-sum-product ?line))
                        (assert (qnum-stats (line ?line)
                                            (length (str-length ?line))
                                            (sum (nth$ 1 ?collec))
                                            (product (nth$ 2 ?collec))))))
               (close foo)))

(defrule generate-qnum-stats
         (declare (salience 1))
         ?f <- (qnum-length ?length)
         =>
         (retract ?f)
         (sum-digit-from-file-to-facts (str-cat "outputs/qnums" ?length)))


(deffacts qnum-bases
          (qnum-length 10) ; has all the numbers below 10 digits as well
          (qnum-length 11)
          (qnum-length 12)
          (qnum-length 13)
          (qnum-length 14)
          (qnum-length 15)
          (qnum-length 16)
          (qnum-length 17)
          (qnum-length 18)
          (qnum-length 19))

(defrule assert-sums-and-products
         ?f <- (qnum-stats (sum ?sum)
                           (product ?product))
         =>
         (assert (sum ?sum)
                 (product ?product)))

(defrule print-found-sums-and-mappings
         (declare (salience -9995))
         (sum ?sum)
         (qnum-stats (sum ?sum)
                     (product ?product)
                     (line ?line))
         =>
         (printout t ?line ": (" ?sum ", " ?product "): (" 
                   (mod ?product 
                        ?sum) 
                   ", " 
                   (mod ?sum
                        ?product) "), " 
                   (or (= ?sum 
                          (mod ?sum 
                               ?product))
                       (= ?sum
                          (mod ?product
                               ?sum)))
                   crlf))

