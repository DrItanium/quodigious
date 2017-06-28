(deffunction not-five 
             (?x)
             (<> ?x 
                 5))
(deffunction entry
             (?i ?j ?k)
             (str-cat { ?i , ?j , ?k , },))
(deffunction perform-compute
             (?s ?p ?n ?f)
             (create$ (+ ?s ?f)
                      (* ?p ?f)
                      (+ ?n ?f)))

(deffunction loop-body
             (?id ?width ?sum ?product ?number)
             (loop-for-count (?i 2 9) do
                             (if (not-five ?i) then
                               (bind ?in
                                     (+ ?number
                                        (integer (** ?i 
                                                     (- ?width 
                                                        1)))))
                               (bind ?ip
                                     (* ?product
                                        ?i))
                               (bind ?is
                                     (+ ?product
                                        ?i))
                               (if (= ?width 2) then
                                 (printout ?id
                                           (entry (expand$ (perform-compute ?is ?ip ?in 2))) 
                                           (entry (expand$ (perform-compute ?is ?ip ?in 3))) 
                                           (entry (expand$ (perform-compute ?is ?ip ?in 4))) 
                                           (entry (expand$ (perform-compute ?is ?ip ?in 6))) 
                                           (entry (expand$ (perform-compute ?is ?ip ?in 7))) 
                                           (entry (expand$ (perform-compute ?is ?ip ?in 8)))
                                           (entry (expand$ (perform-compute ?is ?ip ?in 9))) crlf)
                                 else
                                 (loop-body ?id
                                            (- ?width
                                               1)
                                            ?is
                                            ?ip
                                            ?in)))))
(deffunction doit
             (?path $?inputs)
             (open ?path (bind ?name
                               (gensym*)) "w")
             (progn$ (?input ?inputs) 
                     (printout ?name 
                               "template<>" crlf
                               "inline Triple* getTriples<" ?input ">() noexcept {" crlf)

                     (printout ?name 
                               "static Triple elements[] = {" crlf)
                     (loop-body ?name 
                                ?input 
                                0 1 0)
                     (printout ?name 
                               "};" crlf
                               "return elements;" crlf
                               "}" crlf))
             (close ?name))

(doit "triples2And4.h" 2 4)
(exit)
