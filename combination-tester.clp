(defgeneric generate-combinations)
(defmethod generate-combinations
           ()
           (create$))
(defmethod generate-combinations
           (?a)
           (sym-cat ?a))
(defmethod generate-combinations
           (?a ?b)
           (create$ (sym-cat ?a ?b)
                    (sym-cat ?b ?a)))
(defmethod generate-combinations
           ($?elements)
           (bind ?output
                 (create$))
           (progn$ (?e ?elements)
                   (bind ?others
                         (delete$ ?elements
                          ?e-index
                          ?e-index))
                   (progn$ (?element (generate-combinations (expand$ ?others)))
                           (bind ?output
                            ?output
                            (sym-cat ?e ?element))))
           ?output)

(deffunction assert-potential-combinations
             ($?elements)
             (progn$ (?element (generate-combinations (expand$ ?elements)))
                     (assert (combination ?element))))
