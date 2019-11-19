(define append
  (lambda (L1 L2)
    (if (null? L1)
        L2
        (cons (car L1) (append (cdr L1) L2)))))

(quote (4 5 (6)))
(quote (6 7))
;(append (quote (4 5)) (quote (6 7)))

(cons 3 4)
(cons 6.66 "hi")
(cons 4 (quote (9 0 0)))


;(car (quote (1 2 3)))

;(cdr (quote (1 2 3)))

(define reverse-list
  (lambda (L)
    (if (null? L)
        L
        (append (reverse-list (cdr L)) (cons (car L) (quote ()))))))

;(reverse-list (quote ()))
;(reverse-list (quote (1 2 3 4)))
;(reverse-list (quote (("computer" "science") "is" "not" "awesome")))

;(cdr (quote (1 2)) (quote (2 3)))


;;(null? (quote (1 3 5 2)))
;;(null? "hello")
;;(define x 1)
;;(null? x)
;;(null? (quote ()))
;;(+ 1 2)
;;(+ 1 2 3)
;;(+)
;(+ 4.44 5 -9.7)
;(+ 1 "hello")
