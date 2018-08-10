(defglobal MAIN
           ?*divisor* = (/ 1000000000 128))
(deffunction pow10
             (?value)
             (return (if (<= ?value 0) then 1 else
                       (* 10 (pow10 (- ?value 1))))))

(deffunction generate-numbers
             (?width ?id ?num ?sum ?prod ?depth)
             (if (= ?depth 0) then
               (format ?id
                       "%d %d %d %d%n" 
                       ?width
                       ?sum
                       ?prod
                       ?num)
               else
               (loop-for-count (?i 2 9)
                               (if (<> ?i 5) then
                                   (bind ?iv 
                                         (+ ?num
                                            (* (pow10 (- ?depth 1))
                                               ?i)))
                                   (generate-numbers ?width
                                                     ?id
                                                     ?iv
                                                     (+ ?sum ?i)
                                                     (* ?prod ?i)
                                                     (- ?depth 1))))))

(deffunction gen-range
             (?width)
             (bind ?index
                   0)
             (loop-for-count (?i 2 9) ; 10
                             (if (<> ?i 5) then
                               (bind ?vi 
                                     (* ?i 1000000000))
                               (loop-for-count (?j 2 9) ; 9 
                                               (if (<> ?j 5) then
                                                 (bind ?vj
                                                       (+ ?vi
                                                          (* ?j 100000000)))
                                                 (loop-for-count (?k 2 9) ; 8
                                                                 (if (<> ?k 5) then
                                                                   (bind ?vk
                                                                         (+ ?vj 
                                                                            (* ?k 10000000)))
                                                                   (open (sym-cat data- ?width - ?index)
                                                                         (bind ?id
                                                                               (gensym*))
                                                                         "w")
                                                                   (generate-numbers ?width
                                                                                     ?id
                                                                                     ?vk
                                                                                     (+ ?i ?j ?k)
                                                                                     (* ?i ?j ?k)
                                                                                     8)
                                                                   (bind ?index 
                                                                         (+ ?index 1))
                                                                   (close ?id))))))))

