(deffunction pow10
             (?value)
             (return (if (<= ?value 0) then 1 else
                       (* 10 (pow10 (- ?value 1))))))

(deffunction generate-numbers
             (?id ?num ?sum ?prod ?depth)
             (if (= ?depth 0) then
               (format ?id
                       "%d %d %d%n" 
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
                                 (generate-numbers ?id
                                                   ?iv
                                                   (+ ?sum ?i)
                                                   (* ?prod ?i)
                                                   (- ?depth 1))))))

(deffunction gen-range
             ()
             (loop-for-count (?i 2 9) ; 11
                             (if (<> ?i 5) then
                               (bind ?vi 
                                     (* ?i 1000000000))
                               (loop-for-count (?j 2 9) ; 10
                                               (if (<> ?j 5) then
                                                 (bind ?vj
                                                       (+ ?vi
                                                          (* ?j 100000000)))
                                                 (loop-for-count (?k 2 9) ; 9
                                                                 (if (<> ?k 5) then
                                                                   (bind ?vk
                                                                         (+ ?vj 
                                                                            (* ?k 10000000)))
                                                                   (open (sym-cat data- ?i ?j ?k)
                                                                         (bind ?id
                                                                               (gensym*))
                                                                         "w")
                                                                   (generate-numbers ?id
                                                                                     ?vk
                                                                                     (+ ?i ?j ?k)
                                                                                     (* ?i ?j ?k)
                                                                                     7)
                                                                   (close ?id))
                                                                 ))))))

