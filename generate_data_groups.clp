(deffunction pow10
             (?value)
             (return (if (<= ?value 0) then 1 else
                       (* 10 (pow10 (- ?value 1))))))

(deffunction generate-numbers
             (?id ?num ?sum ?prod ?depth)
             (if (= ?depth 0) then
               (format ?id
                       "%d %d %d " 
                       ?sum
                       ?prod
                       ?num)
               else
               (bind ?next
                     (- ?depth 1))
               (bind ?factor
                     (pow10 ?next))
               (if (= ?depth 1) then
                 (loop-for-count (?i 2 8)
                                 (if (not (<> ?i 2 4 6 8)) then
                                   (generate-numbers ?id
                                                     (+ ?num
                                                        (* ?factor ?i))
                                                     (+ ?sum ?i)
                                                     (* ?prod ?i)
                                                     ?next)))
                 else
                 (loop-for-count (?i 2 9)
                                 (if (<> ?i 5) then
                                   (generate-numbers ?id
                                                     (+ ?num
                                                        (* ?factor ?i))
                                                     (+ ?sum ?i)
                                                     (* ?prod ?i)
                                                     ?next))))))

(deffunction gen-range
             ()
             (loop-for-count (?x 2 9) ; 11
                             (if (<> ?x 5) then
                               (bind ?vx
                                     (* ?x (pow10 10)))
                               (loop-for-count (?i 2 9) ; 10
                                               (if (<> ?i 5) then
                                                 (bind ?vi 
                                                       (+ ?vx
                                                          (* ?i (pow10 9))))
                                                 (loop-for-count (?j 2 9) ; 9
                                                                 (if (<> ?j 5) then
                                                                   (bind ?vj
                                                                         (+ ?vi
                                                                            (* ?j (pow10 8))))
                                                                   (loop-for-count (?k 2 9) ; 8
                                                                                   (if (<> ?k 5) then
                                                                                     (open (sym-cat data- ?x ?i ?j ?k)
                                                                                           (bind ?id
                                                                                                 (gensym*))
                                                                                           "w")
                                                                                     (generate-numbers ?id
                                                                                                       (+ ?vj 
                                                                                                          (* ?k (pow10 7)))
                                                                                                       (+ ?x ?i ?j ?k)
                                                                                                       (* ?x ?i ?j ?k)
                                                                                                       6)
                                                                                     (close ?id))
                                                                                   ))))))))

