(deffunction one-element
             (?sym ?index)
             (sym-cat ?sym ?index))
(deffunction prefix-element
             (?s ?v ?postfix)
             (sym-cat (one-element ?s ?v)
                      ?postfix))
(deffunction two-element
             (?s1 ?s2 ?i1 ?i2)
             (create$ (prefix-element ?s1 ?i1 
                                      (one-element ?s2 ?i2))
                      (prefix-element ?s2 ?i1
                                      (one-element ?s1 ?i2))))
(deffunction three-element
             (?s1 ?s2 ?s3 ?i1 ?i2 ?i3)
             (bind ?output
                   (create$))
             (progn$ (?element (two-element ?s2 ?s3 ?i2 ?i3))
                     (bind ?output
                           ?output
                           (prefix-element ?s1 ?i1
                                           ?element)))
             (progn$ (?element (two-element ?s1 ?s3 ?i2 ?i3))
                     (bind ?output
                           ?output
                           (prefix-element ?s2 ?i1
                                           ?element)))
             (progn$ (?element (two-element ?s1 ?s2 ?i2 ?i3))
                     (bind ?output
                           ?output
                           (prefix-element ?s3 ?i1
                                           ?element)))
             ?output)
(deffunction four-element
             (?s1 ?s2 ?s3 ?s4 ?i1 ?i2 ?i3 ?i4)
             (bind ?output
                   (create$))
             (progn$ (?element (three-element ?s2 ?s3 ?s4 ?i2 ?i3 ?i4))
                     (bind ?output
                           ?output
                           (prefix-element ?s1 ?i1
                                           ?element)))
             (progn$ (?element (three-element ?s1 ?s3 ?s4 ?i2 ?i3 ?i4))
                     (bind ?output
                           ?output
                           (prefix-element ?s2 ?i1
                                           ?element)))
             (progn$ (?element (three-element ?s1 ?s2 ?s4 ?i2 ?i3 ?i4))
                     (bind ?output
                           ?output
                           (prefix-element ?s3 ?i1
                                           ?element)))
             (progn$ (?element (three-element ?s1 ?s2 ?s3 ?i2 ?i3 ?i4))
                     (bind ?output
                           ?output
                           (prefix-element ?s4 ?i1
                                           ?element)))
             ?output)


(deffunction five-element
             (?s1 ?s2 ?s3 ?s4 ?s5 ?i1 ?i2 ?i3 ?i4 ?i5)
             (bind ?output
                   (create$))
             (progn$ (?element (four-element ?s2 ?s3 ?s4 ?s5 ?i2 ?i3 ?i4 ?i5))
                     (bind ?output
                           ?output
                           (prefix-element ?s1 ?i1
                                           ?element)))
             (progn$ (?element (four-element ?s1 ?s3 ?s4 ?s5 ?i2 ?i3 ?i4 ?i5))
                     (bind ?output
                           ?output
                           (prefix-element ?s2 ?i1
                                           ?element)))
             (progn$ (?element (four-element ?s1 ?s2 ?s4 ?s5 ?i2 ?i3 ?i4 ?i5))
                     (bind ?output
                           ?output
                           (prefix-element ?s3 ?i1
                                           ?element)))
             (progn$ (?element (four-element ?s1 ?s2 ?s3 ?s5 ?i2 ?i3 ?i4 ?i5))
                     (bind ?output
                           ?output
                           (prefix-element ?s4 ?i1
                                           ?element)))
             (progn$ (?element (four-element ?s1 ?s2 ?s3 ?s4 ?i2 ?i3 ?i4 ?i5))
                     (bind ?output
                           ?output
                           (prefix-element ?s5 ?i1
                                           ?element)))
             ?output)
(deffunction six-element
             (?s1 ?s2 ?s3 ?s4 ?s5 ?s6 ?i1 ?i2 ?i3 ?i4 ?i5 ?i6)
             (bind ?output
                   (create$))
             (progn$ (?element (five-element ?s2 ?s3 ?s4 ?s5 ?s6 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s1 ?i1
                                           ?element)))
             (progn$ (?element (five-element ?s1 ?s3 ?s4 ?s5 ?s6 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s2 ?i1
                                           ?element)))
             (progn$ (?element (five-element ?s1 ?s2 ?s4 ?s5 ?s6 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s3 ?i1
                                           ?element)))
             (progn$ (?element (five-element ?s1 ?s2 ?s3 ?s5 ?s6 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s4 ?i1
                                           ?element)))
             (progn$ (?element (five-element ?s1 ?s2 ?s3 ?s4 ?s6 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s5 ?i1
                                           ?element)))
             (progn$ (?element (five-element ?s1 ?s2 ?s3 ?s4 ?s5 ?i2 ?i3 ?i4 ?i5 ?i6))
                     (bind ?output
                           ?output
                           (prefix-element ?s6 ?i1
                                           ?element)))
             ?output)

(deffunction assert-potential-combinations
             (?a ?b ?c ?d ?e ?f)
             (progn$ (?element (six-element ?a ?b ?c ?d ?e ?f 1 2 3 4 5 6))
                     (assert (combination ?element))))
