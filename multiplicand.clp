(deffunction assert-facts
             (?width ?base)
             (progn$ (?i (create$ 2 3 4 6 7 8 9))
                     (assert (value ?width 
                                    (* ?base ?i)))))
(deffunction open-file-and-assert
             (?width ?path)
             (bind ?file
                   (gensym*))
             (if (open ?path
                       ?file
                       "r") then
               (while (neq (bind ?entry
                                 (read ?file))
                           EOF) do
                      (assert-facts ?width
                                    ?entry))
               (close ?file)
               else
               (printout werror
                         "Couldn't open " 
                         ?path " for reading!" crlf
                         tab "Make sure it exists or permissions are correct!" crlf)))
(deffacts initial-setup
          (setup output 15 to research/uniq_prod15)
          (setup output 16 to research/uniq_prod16)
          (setup output 17 to research/uniq_prod17)
          (setup output 18 to research/uniq_prod18)
          (setup output 19 to research/uniq_prod19)
          (stop at 19)
          (inspect 10 research/uniq_prod10))
(defrule load-file
         (declare (salience 10000))
         ?f <- (inspect ?width ?path)
         =>
         (retract ?f)
         (open-file-and-assert ?width
                               ?path))


(defrule setup-output
         (declare (salience 10000))
         ?f <- (setup output ?width to ?path)
         =>
         (retract ?f)
         (if (open ?path
                   (bind ?id 
                         (gensym*))
                   "w") then
           (assert (output ?width to ?id))
           else
           (printout werror
                     "couldn't open " ?path
                     " for writing" crlf)))

(defrule compute-next-width
         (declare (salience 1))
         (value ?width
                ?value)
         (stop at ?stop-value)
         (test (< ?width
                  ?stop-value))
         =>
         (assert-facts (+ ?width 
                          1)
                       ?value))

(defrule output-to-target-width
         (value ?width
                ?value)
         (output ?width
                 to
                 ?id)
         =>
         (printout ?id
                   ?value crlf))

(defrule close-target-width
         (declare (salience -10000))
         ?f <- (output ? to ?id)
         =>
         (retract ?f)
         (close ?id))


(reset)
(run)
(exit)
